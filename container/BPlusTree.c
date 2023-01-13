#include "BPlusTree.h"
#include "CUtils/container/array.h"

typedef enum _BPlusEntryType {
	kIndex,
	kLeaf,
} BPlusEntryType;

static BPlusEntry* BPlusCreateIndexEntry(int m) {
	// child Ӧ����m����������������һ��
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) - sizeof(ListEntry) + (m - 1) * sizeof(BPlusIndexInternalEntry) + sizeof(struct _BPlusIndexEntry*));
	entry->type = kIndex;
	entry->parent = NULL;
	for (int i = 0; i < m; i++) {
		entry->indexInternalEntry[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}

static BPlusEntry* BPlusCreateLeafEntry(int m) {
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) + m * sizeof(BPlusLeafInternalEntry));
	entry->type = kLeaf;
	entry->parent = NULL;
	// ListEntryInit(&entry->leafListEntry);
	for (int i = 0; i < m - 1; i++) {
		// entry->entry[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}

/*
* û����ȷ��׼��������key����
* 
* ͨ������£����ڵ��ڲ��ڵ������������ӽڵ������ӽڵ�
* ��Ĭ����key[0] - child[1]��key[1] - child[2]...
*         3     |    6    |    8
*      /        \         \        \
*   1|2         4|5        7        10|11
* �����滭��ʾ����Ĭ����3��Ӧ4|5��6��Ӧ7��8��Ӧ10|11
*/

/*
* ��ȡ���ֵܽڵ㣬û���򷵻�NULL
*/
static BPlusEntry* BPlusGetLeftSiblingEntry(BPlusTree* tree, BPlusEntry* entry, int leftParentIndex) {
	BPlusEntry* parent = entry->parent;
	if (!parent) {
		return NULL;
	}
	if (leftParentIndex > -1) {
		return parent->indexInternalEntry[leftParentIndex].child;
	}
	return NULL;
}

/*
* ��ȡ���ֵܽڵ㣬û���򷵻�NULL
*/
static BPlusEntry* BPlusGetRightSiblingEntry(BPlusTree* tree, BPlusEntry* entry, int leftParentIndex) {
	BPlusEntry* parent = entry->parent;
	if (!parent) {
		return NULL;
	}
	if (leftParentIndex < parent->count) {
		return parent->indexInternalEntry[leftParentIndex + 2].child;
	}
	return NULL;
}

/*
* ���ڲ��ڵ�����ָ��λ�ò���key���ӽڵ�(�������ڵ�ʱ)
*/
static void BPlusInsertInternalEntry(BPlusEntry* entry, int keyIndex, int key, BPlusEntry* child, bool rightChild) {
	if (entry->type == kLeaf) {
		BPlusEntry* leaf = entry;
		for (int j = leaf->count - 1; j >= keyIndex; j--) {
			leaf->leafInternalEntry[j + 1].key = leaf->leafInternalEntry[j].key;
		}
		leaf->leafInternalEntry[keyIndex].key = key;
		leaf->count++;
	}
	else {
		BPlusEntry* index = entry;
		for (int j = index->count - 1; j >= keyIndex; j--) {
			index->indexInternalEntry[j + 1].key = index->indexInternalEntry[j].key;
			index->indexInternalEntry[j + 2].child = index->indexInternalEntry[j + 1].child;
		}
		index->indexInternalEntry[keyIndex].key = key;
		child->parent = index;
		if (!rightChild) {
			// �����ӽڵ㣬��Ųһ������ keyIndex-- ʹ��ָ�����ӽڵ�
			index->indexInternalEntry[keyIndex + 1].child = index->indexInternalEntry[keyIndex].child;
			keyIndex--;
		}
		index->indexInternalEntry[keyIndex + 1].child = child;
		index->count++;
	}
}

/*
* ���ڲ��ڵ�������ɾ��ָ���������ڲ��ڵ�
* ���ر�ɾ���ڲ��ڵ���ӽڵ�
*/
static BPlusEntry* BPlusDeleteInternalEntry(BPlusEntry* entry, int keyIndex, int* key, bool rightChild) {
	if (entry->type == kLeaf) {
		BPlusEntry* leaf = entry;
		if (key) {
			*key = leaf->leafInternalEntry[keyIndex].key;
		}
		for (int i = keyIndex + 1; i < leaf->count; i++) {
			leaf->leafInternalEntry[i - 1].key = leaf->leafInternalEntry[i].key;
		}
		leaf->count--;
		return NULL;
	}
	else {
		BPlusEntry* index = entry;
		if (key) {
			*key = index->indexInternalEntry[keyIndex].key;
		}
		BPlusEntry* deleteEntry;
		if (rightChild) {
			deleteEntry = index->indexInternalEntry[keyIndex + 1].child;
			for (int i = keyIndex + 1; i < index->count; i++) {
				index->indexInternalEntry[i - 1].key = index->indexInternalEntry[i].key;
				index->indexInternalEntry[i].child = index->indexInternalEntry[i + 1].child;
			}
		} else {
			deleteEntry = index->indexInternalEntry[keyIndex].child;
			for (int i = keyIndex + 1; i < index->count; i++) {
				index->indexInternalEntry[i - 1].key = index->indexInternalEntry[i].key;
				index->indexInternalEntry[i - 1].child = index->indexInternalEntry[i].child;
			}
			index->indexInternalEntry[index->count-1].child = index->indexInternalEntry[index->count].child;
		}
		index->count--;
		return deleteEntry;
	}
}

/*
* ���ѽڵ�
* ͬһ��Ҷ�ڵ��η��ѻᵼ���ظ���key������
* ���Ҷ�ڵ㲻������ͬ��key����ô�ǲ����
* ��Ϊ���Ѻ�ѡ���ҽڵ�������ڲ��ڵ��key��Ϊ������key
* ���ʱ��������ô�����ڲ��ڵ㶼������뵽�ýڵ������ڲ��ڵ�����(����С�Ļᱻ�ֵ����ڵ㣬��Ϊ�������ڲ��ڵ�key���ڸ��ڲ��ڵ�)���ýڵ��ٷ���Ҳ�Ͳ����������ڲ��ڵ��ٴ������Ŀ�����
*/
static bool BPlusInsertEntry(BPlusTree* tree, BPlusEntry* entry, int key, BPlusEntry* rightChild);
static void BPlusSplitEntry(BPlusTree* tree, BPlusEntry* entry, int insertIndex, int key, BPlusEntry* rightChild) {
	BPlusEntry* newEntry = NULL;
	int newCount;
	if (entry->type == kLeaf) {

		newEntry = BPlusCreateLeafEntry(tree->m);

		ListInsertHead(&entry->leafListEntry, &newEntry->leafListEntry);

		// ԭ�ط���˼·��mid��δ������ڲ��ڵ�Ҳ���ϣ��ü���newCount��4�ײ����4�ڵ����2(��2��2)��5�ײ����5�ڵ㻹��2(��2��3)
		// ������ǰ����Ҳ�Ӧ���ж��ٸ��ڲ��ڵ㣬������ȥ���м��������ڲ��ڵ����ʹ�����һ�εĿ�����û�����ٲ��뵽���
		int mid = tree->m / 2;
		newCount = entry->count + 1 - mid;		// +1����Ϊ���ʱ��entry->count��û�а�δ�����ڲ��ڵ�Ҳ����
		int i = newCount - 1, j = entry->count - 1;
		bool insert = false;
		for (; i >= 0; i--, j--) {
			if (!insert && j+1 == insertIndex) {		// ����j+1����Ϊ��ѭ����ʱ��j��û�а�δ�����ڲ��ڵ�Ҳ����
				entry->count++;
				newEntry->leafInternalEntry[i].key = key;
				j++;		// j����
				insert = true;
				continue;
			}
			newEntry->leafInternalEntry[i].key = entry->leafInternalEntry[j].key;
		}

		entry->count -= newCount;

		if (!insert) {
			// ���ڲ��ڵ㻹û�в��룬�������
			BPlusInsertInternalEntry(entry, insertIndex, key, rightChild, true);
		}
		
		// ��mid�õ������ڲ��ڵ�
		key = newEntry->leafInternalEntry[0].key;
	}
	else {
		newEntry = BPlusCreateIndexEntry(tree->m);

		// ԭ�ط���˼·��mid��δ������ڲ��ڵ�ͼ����������ڲ��ڵ㶼���ϣ��ü���newCount��4�ײ����4�ڵ����4/2=2(��1��2)��5�ײ����5�ڵ�Ҳ��2(��2��2)������һ������Ϊ������Ҳ������

		// �Ƚ���벿�ֿ������½ڵ㣬����м������������Ĳ��룬�Ǿ�һ�����룬����midkey��entry->indexData[entry->count-1]����Ϊ�Ҳ����������ǰ��õģ��������һ���ŵ����
		//  1 2       3 4
		// / |  |      |  \	
		// ��ʱ�½ڵ�ȱ����һ�����ӣ���������ѡ�þɽڵ��ĩβ�ڲ��ڵ���Ϊ�����ڲ��ڵ㣬�ʾɽڵ������ĩβ�ڲ��ڵ���������Ϳ��Թҽӵ��½ڵ���

		int mid = (tree->m-1) / 2;
		newCount = entry->count - mid;		// ���ʱ��entry->count��û�а�δ�����ڲ��ڵ�Ҳ���ϣ����ǻ�����һ���ڲ��ڵ㣬�����ʲ�����
		int i = newCount - 1, j = entry->count - 1;
		bool insert = false;
		for (; i >= 0; i--, j--) {
			if (!insert && j+1 == insertIndex) {		// ����j+1����Ϊ��ѭ����ʱ��j��û�а�δ�����ڲ��ڵ�Ҳ����
				entry->count++;
				newEntry->indexInternalEntry[i].key = key;
				newEntry->indexInternalEntry[i+1].child = rightChild;
				rightChild->parent = newEntry;
				j++;		// j����
				insert = true;
				continue;
			}
			newEntry->indexInternalEntry[i].key = entry->indexInternalEntry[j].key;
			newEntry->indexInternalEntry[i+1].child = entry->indexInternalEntry[j+1].child;
			newEntry->indexInternalEntry[i+1].child->parent = newEntry;
		}
		entry->count -= newCount;

		if (!insert) {
			// ���ڲ��ڵ㻹û�в��룬�������
			BPlusInsertInternalEntry(entry, insertIndex, key, rightChild, true);
		}

		// ����entry->indexDataĩβ�õ������ڲ��ڵ㣬����ժ��
		entry->count--;
		key = entry->indexInternalEntry[entry->count].key;

		// entry���ҵ����ӽ���newEntry
		newEntry->indexInternalEntry[0].child = entry->indexInternalEntry[entry->count+1].child;
		newEntry->indexInternalEntry[0].child->parent = newEntry;
	}
	newEntry->count = newCount;

	// ���ѳ����ڲ��ڵ����ϴ���
	if (entry->parent == NULL) {
		BPlusEntry* newIndex = BPlusCreateIndexEntry(tree->m);
		entry->parent = newIndex;
		newIndex->indexInternalEntry[0].child = entry;
		BPlusInsertInternalEntry(newIndex, 0, key, newEntry, true);
		tree->root = newIndex;
	}
	else {
		BPlusInsertEntry(tree, entry->parent, key, newEntry);
	}
}

/*
* ����ڵ�
*/
static bool BPlusInsertEntry(BPlusTree* tree, BPlusEntry* entry, int key, BPlusEntry* rightChild) {
	if (entry->count == 0) {
		BPlusInsertInternalEntry(entry, 0, key, rightChild, true);
		return true;
	}
	int insertIndex;
	int res;
	if (entry->type == kLeaf) {
		insertIndex = BinarySearch_Range_CustomM(entry->leafInternalEntry, BPlusLeafInternalEntry, key, 0, entry->count - 1, &key, MemoryCmpR);
		res = MemoryCmpR(&entry->leafInternalEntry[insertIndex].key, &key, sizeof(key));
	}
	else {
		insertIndex = BinarySearch_Range_CustomM(entry->indexInternalEntry, BPlusIndexInternalEntry, key, 0, entry->count - 1, &key, MemoryCmpR);
		res = MemoryCmpR(&entry->indexInternalEntry[insertIndex].key, &key, sizeof(key));
	}
	//if (res == 0) {		// ���������ͬ�Ľڵ�
	//	return false;
	//}
	if (res < 0) {
		insertIndex++;		// ������нڵ㶼С��key���Ǿ�׷�ӵ���β��
	}

	if (entry->count < tree->m - 1) {
		// �п����λ�ò���
		BPlusInsertInternalEntry(entry, insertIndex, key, rightChild, true);
		return true;
	}

	// û�ж���λ�ã���Ҫ�������Ϻϲ�
	BPlusSplitEntry(tree, entry, insertIndex, key, rightChild);
	return true;
}

/*
* �ϲ��ڵ�
*   2
* 1   3
* �������ҵĹ�ϵ����
*/
static bool BPlusDeleteEntry(BPlusTree* tree, BPlusEntry* entry, int deleteIndex, Array* stack);
static void BPlusMergeEntry(BPlusTree* tree, BPlusEntry* left, BPlusEntry* right, int commonParentIndex, Array* stack) {
	if (left->type == kLeaf) {
		// ��Ҷ�ӽڵ㣬��right����left�У���ɾ��right�ĸ������ڲ��ڵ�
		for (int i = 0; i < right->count; i++) {
			left->leafInternalEntry[left->count++].key = right->leafInternalEntry[i].key;
		}
	} else {
		// �������ڵ㣬��������ɾ���ĸ������ڲ��ڵ�(�ӽڵ㶪������Ϊ�������ӽڵ��ָ�������)��right�����뵽left�У�ɾ��right�ĸ������ڲ��ڵ�
		left->indexInternalEntry[left->count++].key = right->parent->indexInternalEntry[commonParentIndex].key;
		// �ҽڵ����һ���ӽڵ㣬��Ϊ���ڵ㲻��Ҫ���ӽڵ��½������Կ���ֱ�Ӳ�����ڵ�β��
		left->indexInternalEntry[left->count].child = right->indexInternalEntry[0].child;		
		right->indexInternalEntry[0].child->parent = left;
		for (int i = 0; i < right->count; i++) {
			left->indexInternalEntry[left->count++].key = right->indexInternalEntry[i].key;
			left->indexInternalEntry[left->count].child = right->indexInternalEntry[i+1].child;
			right->indexInternalEntry[i + 1].child->parent = left;
		}
	}
	BPlusDeleteEntry(tree, left->parent, commonParentIndex, stack);
	DeleteObject_(right);
}

/*
* ɾ��ָ���ڵ�
*/
static bool BPlusDeleteEntry(BPlusTree* tree, BPlusEntry* entry, int deleteIndex, Array* stack) {
	BPlusDeleteInternalEntry(entry, deleteIndex, NULL, true);
	if (entry->count >= (tree->m-1) / 2) {
		return true;
	}

	int* leftParentIndex = ArrayPopTail(stack);
	if (!leftParentIndex) {
		// û�и��ڵ���Ѿ������ڵ��ˣ���Ҷ�ӽڵ���������������ڵ����ж��Ƿ�û���κ��ӽڵ��ˣ��������������һ���ӽڵ�Ϊ���ڵ㣬����ֱ�ӽ���
		if (entry->type == kIndex && entry->count == 0) {
			BPlusEntry* child = entry->indexInternalEntry[0].child;
			child->parent = NULL;
			tree->root = child;
			DeleteObject_(entry);
		}
		return true;
	}

	BPlusEntry* sibling = BPlusGetLeftSiblingEntry(tree, entry, *leftParentIndex);
	bool leftSibling = true;
	if (!sibling) {
		sibling = BPlusGetRightSiblingEntry(tree, entry, *leftParentIndex);
		leftSibling = false;
	}
	if (sibling->count > (tree->m-1) / 2) {
		// ���ֵܽ�ڵ�
		int key;
		if (entry->type == kLeaf) {
			// Ҷ�ӽڵ㴦��ϼ򵥣�����ֱ���ƶ�
			if (leftSibling) {
				// �����ֵܽڵ��ĩβ���ڲ��ڵ���뵽��ǰ�ڵ��ͷ�������¸��ڲ��ڵ�keyΪ������key
				BPlusEntry* child = BPlusDeleteInternalEntry(sibling, sibling->count - 1, &key, true);
				BPlusInsertInternalEntry(entry, 0, key, child, true);
			}
			else {
				// �����ֵܽڵ��ͷ�����ڲ��ڵ���뵽��ǰ�ڵ��β�������¸��ڲ��ڵ�keyΪ���ֵܵ������ڲ��ڵ�
				BPlusEntry* child = BPlusDeleteInternalEntry(sibling, 0, &key, true);
				BPlusInsertInternalEntry(entry, entry->count, key, child, true);
				key = sibling->leafInternalEntry[0].key;		// �ҽڵ��ͷ�ڲ��ڵ�key�������ú͹�ͬ���ڵ����(��ʱ��������ȵ�key�ܵ���ߣ��ͻᵼ���Ҳ���)�����key����Ϊ�µ����ڲ��ڵ�����õ�
				++*leftParentIndex;		// ��ǰ�ڵ�����ڵ㣬Ҫ�����ֵܽڵ�Ĺ�ͬ���ڵ������£�����++
			}
			
		} else {
			// �����ڵ㴦��ϸ��ӣ���Ҫ�½����ڵ���е�ǰ�ڵ���ֵܽڵ�Ĺ�ͬ�ڲ��ڵ㣬�����ֵܽڵ㵽��ͬ���ڵ�(������)
			if (leftSibling) {
				// ���ֵܽڵ��ĩβ�ڲ��ڵ����������ڵ��ͷ�������ڵ�Ķ�Ӧ�ڲ��ڵ��½�����ǰ�ڵ��ͷ���������ڲ��ڵ������ӽڵ�����½��ĸ��ڵ��ڲ��ڵ���Ҳ�
				BPlusEntry* rightChild = BPlusDeleteInternalEntry(sibling, sibling->count - 1, &key, true);
				BPlusInsertInternalEntry(entry, 0, entry->parent->indexInternalEntry[*leftParentIndex].key, rightChild, false);
			} else {
				// ���ֵܽڵ��ͷ�ڲ��ڵ����������ڵ��ͷ�������ڵ�Ķ�Ӧ�ڲ��ڵ��½�����ǰ�ڵ��β���������ڲ��ڵ������ӽڵ�����½��ĸ��ڵ��ڲ��ڵ�����
				BPlusEntry* leftChild = BPlusDeleteInternalEntry(sibling, 0, &key, false);
				++*leftParentIndex;		// ��ǰ�ڵ�����ڵ㣬Ҫ�����ֵܽڵ�Ĺ�ͬ���ڵ������£�����++
				BPlusInsertInternalEntry(entry, entry->count, entry->parent->indexInternalEntry[*leftParentIndex].key, leftChild, true);
			}
		}
		entry->parent->indexInternalEntry[*leftParentIndex].key = key;
		return true;
	}

	// �ֵܽڵ㲻���裬��Ҫ�ϲ�(�ϲ���Ҳ���ᳬ��m-1����Ϊһ�߲���m-1��һ�룬һ����m-1��һ�룬�������ڵ�ϲ�Ҳ�㹻�½�һ�����ڲ��ڵ�)
	if (leftSibling) {
		BPlusMergeEntry(tree, sibling, entry, *leftParentIndex, stack);
	} else {
		BPlusMergeEntry(tree, entry, sibling, *leftParentIndex + 1, stack);		// Ҫ��ͬ������
	}
	return true;
}

/*
* ����key���ҵ�ָ����Ҷ�ӽڵ�
* stack����ÿһ��ڵ�ĸ��ڵ��ڲ��ڵ�(��)����
*/
static BPlusEntry* BPlusTreeFindLeaf(BPlusTree* tree, int key, Array* stack) {
	BPlusEntry* cur = tree->root;
	while (cur && cur->type == kIndex) {
		//       4        8         12
		//    /       |        |       \
		// 3  4      6  7      10        15
		int i = BinarySearch_Range_CustomM(cur->indexInternalEntry, BPlusIndexInternalEntry, key, 0, cur->count - 1, &key, MemoryCmpR);
		
		if (key < cur->indexInternalEntry[i].key) {
			cur = cur->indexInternalEntry[i].child;
		}
		else {
			// ��ȵ�key���ұߣ���Ϊ���ѵ���������Ҷ�ӵĵ�һ���ڵ�������
			cur = cur->indexInternalEntry[++i].child;
		}
		if (stack) {
			i--;		// ������ջ�ĸ��ڵ��ڲ��ڵ�����
			ArrayPushTail(stack, &i);
		}
	}
	return cur;
}



void BPlusTreeInit(BPlusTree* tree, int m) {
	if (m < 2) {
		m = 2;
	}
	tree->root = BPlusCreateLeafEntry(m);
	ListHeadInit(&tree->listHead);
	ListInsertHead(&tree->listHead, &tree->root->leafListEntry);
	tree->m = m;
}

bool BPlusTreeFind(BPlusTree* tree, int key) {
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key, NULL);
	int index = BinarySearch_CustomM(leaf->leafInternalEntry, BPlusLeafInternalEntry, key, 0, leaf->count - 1, &key, MemoryCmpR);
	if (index == -1) {
		return false;
	}
	return true;
}

bool BPlusTreeInsert(BPlusTree* tree, int key) {
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key, NULL);
	return BPlusInsertEntry(tree, leaf, key, NULL);
}

bool BPlusTreeDelete(BPlusTree* tree, int key) {
	Array stack;
	ArrayInit(&stack, 16, sizeof(int));
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key, &stack);

	int deleteIndex = BinarySearch_CustomM(leaf->leafInternalEntry, BPlusLeafInternalEntry, key, 0, leaf->count - 1, &key, MemoryCmpR);
	if (deleteIndex == -1) {
		ArrayRelease(&stack);
		return false;
	}
	bool success = BPlusDeleteEntry(tree, leaf, deleteIndex, &stack);

	ArrayRelease(&stack);

	return success;
}
