#include "BPlusTree.h"

static BPlusEntry* BPlusCreateIndexEntry(int m) {
	// child Ӧ����m����������������һ��
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) - sizeof(ListEntry) + (m - 1) * sizeof(BPlusIndexData) + sizeof(struct _BPlusIndexEntry*));
	entry->type = kIndex;
	entry->parent = NULL;
	for (int i = 0; i < m; i++) {
		entry->indexData[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}

static BPlusEntry* BPlusCreateLeafEntry(int m) {
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) + m * sizeof(BPlusLeafData));
	entry->type = kLeaf;
	entry->parent = NULL;
	// ListEntryInit(&entry->leafListEntry);
	for (int i = 0; i < m - 1; i++) {
		// entry->entry[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}

static void BPlusInsertToArray(BPlusEntry* entry, int keyIndex, int key, BPlusEntry* newChild) {
	if (entry->type == kLeaf) {
		BPlusEntry* leaf = entry;
		for (int j = leaf->count - 1; j >= keyIndex; j--) {
			leaf->leafData[j + 1].key = leaf->leafData[j].key;
		}
		leaf->leafData[keyIndex].key = key;
		leaf->count++;
	}
	else {
		BPlusEntry* index = entry;
		for (int j = index->count - 1; j >= keyIndex; j--) {
			index->indexData[j + 1].key = index->indexData[j].key;
			index->indexData[j + 2].child = index->indexData[j + 1].child;
		}
		index->indexData[keyIndex].key = key;
		newChild->parent = index;
		index->indexData[keyIndex + 1].child = newChild;
		index->count++;
	}
}

static bool BPlusInsertToEntry(BPlusTree* tree, BPlusEntry* entry, int key, BPlusEntry* newChild);
static void BPlusSplitEntry(BPlusTree* tree, BPlusEntry* entry, int insertIndex, int key, BPlusEntry* newChild) {
	// ����ѡȡ�м�Ľڵ�[mid]��Ϊ�����ڵ㣬����������������ڵ�
	BPlusEntry* newEntry = NULL;
	int newCount;
	if (entry->type == kLeaf) {

		newEntry = BPlusCreateLeafEntry(tree->m);

		ListInsertHead(&entry->leafListEntry, &newEntry->leafListEntry);

		// ԭ�ط���˼·��mid��δ����Ľڵ�Ҳ���ϣ��ü���newCount��4�ڵ����2(��2��2)��5�ڵ㻹��2(��2��3)
		int mid = tree->m / 2;
		newCount = entry->count + 1 - mid;		// +1����Ϊ���ʱ��entry->count��û�а�δ����ڵ�Ҳ����
		int i = newCount - 1, j = entry->count - 1;
		bool insert = false;
		for (; i >= 0; i--, j--) {
			if (!insert && j+1 == insertIndex) {		// ����j+1����Ϊ��ѭ����ʱ��j��û�а�δ����ڵ�Ҳ����
				entry->count++;
				newEntry->leafData[i].key = key;
				j++;		// j����
				insert = true;
				continue;
			}
			newEntry->leafData[i].key = entry->leafData[j].key;
		}

		entry->count -= newCount;

		if (!insert) {
			// ������û�в��룬�������
			BPlusInsertToArray(entry, insertIndex, key, newChild);
		}
		
		// ��mid�õ������ڵ�
		key = newEntry->leafData[0].key;
	}
	else {
		newEntry = BPlusCreateIndexEntry(tree->m);

		// ԭ�ط���˼·��mid��δ����Ľڵ�ͼ��������Ľڵ㶼���ϣ��ü���newCount��4�ڵ����2(��1��2)��5�ڵ�Ҳ��2(��2��2)������һ������Ϊ������Ҳ������

		// �Ƚ���벿�ֿ������½ڵ㣬����м������������Ĳ��룬�Ǿ�һ�����룬����midkey��entry->indexData[entry->count-1]����Ϊ�Ҳ����������ǰ��õģ��������һ���ŵ����
		//  1 2      3 4
		// / |  |     |  \	
		// ��ʱ�½ڵ�ȱ����һ�����ӣ���������ѡ������ĩβ�ڵ���Ϊ�����ڵ㣬��ĩβ�ڵ���������Ϳ��Թҽӵ��½ڵ���

		int mid = (tree->m-1) / 2;
		newCount = entry->count - mid;		// ���ʱ��entry->count��û�а�δ����ڵ�Ҳ���ϣ���������mid���������ʲ���
		int i = newCount - 1, j = entry->count - 1;
		bool insert = false;
		for (; i >= 0; i--, j--) {
			if (!insert && j+1 == insertIndex) {		// ����j+1����Ϊ��ѭ����ʱ��j��û�а�δ����ڵ�Ҳ����
				entry->count++;
				newEntry->indexData[i].key = key;
				newEntry->indexData[i+1].child = newChild;
				newChild->parent = newEntry;
				j++;		// j����
				insert = true;
				continue;
			}
			newEntry->indexData[i].key = entry->indexData[j].key;
			newEntry->indexData[i+1].child = entry->indexData[j+1].child;
			newEntry->indexData[i+1].child->parent = newEntry;
		}
		entry->count -= newCount;

		if (!insert) {
			// ������û�в��룬�������
			BPlusInsertToArray(entry, insertIndex, key, newChild);
		}

		// ����entry->indexDataĩβ�õ������ڵ㣬����ժ��
		entry->count--;
		key = entry->indexData[entry->count].key;

		// entry���ҵ����ӽ���newEntry
		newEntry->indexData[0].child = entry->indexData[entry->count+1].child;
		newEntry->indexData[0].child->parent = newEntry;
	}
	newEntry->count = newCount;

	// ���ѳ��ĸ��ڵ����ϴ���
	if (entry->parent == NULL) {
		BPlusEntry* newIndex = BPlusCreateIndexEntry(tree->m);
		entry->parent = newIndex;
		newIndex->indexData[0].child = entry;
		BPlusInsertToArray(newIndex, 0, key, newEntry);
		tree->root = newIndex;
	}
	else {
		BPlusInsertToEntry(tree, entry->parent, key, newEntry);
	}
}

static bool BPlusInsertToEntry(BPlusTree* tree, BPlusEntry* entry, int key, BPlusEntry* newChild) {
	if (entry->count == 0) {
		BPlusInsertToArray(entry, 0, key, newChild);
		return true;
	}
	int insertIndex;
	int res;
	if (entry->type == kLeaf) {
		insertIndex = BinarySearch_Range_CustomM(entry->leafData, BPlusLeafData, key, 0, entry->count - 1, &key, MemoryCmpR);
		res = MemoryCmpR(&entry->leafData[insertIndex].key, &key, sizeof(key));
	}
	else {
		insertIndex = BinarySearch_Range_CustomM(entry->indexData, BPlusIndexData, key, 0, entry->count - 1, &key, MemoryCmpR);
		res = MemoryCmpR(&entry->indexData[insertIndex].key, &key, sizeof(key));
	}
	//if (res == 0) {		// ���������ͬ�Ľڵ�
	//	return false;
	//}
	if (res < 0) {
		insertIndex++;		// ������нڵ㶼С��key���Ǿ�׷�ӵ���β��
	}

	if (entry->count < tree->m - 1) {
		// �п����λ�ò���
		BPlusInsertToArray(entry, insertIndex, key, newChild);
		return true;
	}

	// û�ж���λ�ã���Ҫ�������Ϻϲ�
	BPlusSplitEntry(tree, entry, insertIndex, key, newChild);
	return true;
}

static BPlusEntry* BPlusTreeFindLeaf(BPlusTree* tree, int key) {
	BPlusEntry* cur = tree->root;
	while (cur && cur->type == kIndex) {
		//       4        8         12
		//    /       |        |       \
		// 3  4      6  7      10        15
		int i = BinarySearch_Range_CustomM(cur->indexData, BPlusIndexData, key, 0, cur->count - 1, &key, MemoryCmpR);
		if (key < cur->indexData[i].key) {
			cur = cur->indexData[i].child;
		}
		else {
			// ��ȵ�key���ұߣ���Ϊ���ѵ���������Ҷ�ӵĵ�һ���ڵ�������
			cur = cur->indexData[i + 1].child;
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

bool BPlusTreeInsert(BPlusTree* tree, int key) {
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key);
	return BPlusInsertToEntry(tree, leaf, key, NULL);
}

bool BPlusTreeFind(BPlusTree* tree, int key) {
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key);
	int index = BinarySearch_CustomM(leaf->leafData, BPlusLeafData, key, 0, leaf->count - 1, &key, MemoryCmpR);
	if (index == -1) {
		return false;
	}
	return true;
}