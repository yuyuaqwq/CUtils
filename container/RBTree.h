#ifndef CUTILS_RBTREE_H_
#define CUTILS_RBTREE_H_

#include "CUtils/container/object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* ��234��Ϊԭ�͵ĺ����ʵ��
*/
typedef enum _RBColor {
	kBlack,
	kRed,
} RBColor;
typedef struct _RBEntry {
	struct _RBEntry* parent;
	struct _RBEntry* left;
	struct _RBEntry* right;
	RBColor color;
} RBEntry;

typedef struct _RBTree {
	RBEntry* root;
	// bool smallByteOrder;
	int objSize;
	int entryFieldOffset;
	int keyFieldOffset;
	int keyFieldSize;
} RBTree;


/*
* ��������
*/
static RBEntry* RotateLeft(RBEntry* subRoot) {
	RBEntry* newSubRoot = subRoot->right;
	if (newSubRoot == NULL) {
		return subRoot;
	}

	newSubRoot->parent = subRoot->parent;
	if (subRoot->parent) {
		if (subRoot->parent->left == subRoot) {
			subRoot->parent->left = newSubRoot;
		}
		else {
			subRoot->parent->right = newSubRoot;
		}
	}
	subRoot->parent = newSubRoot;

	subRoot->right = newSubRoot->left;
	if (subRoot->right) {
		subRoot->right->parent = subRoot;
	}

	newSubRoot->left = subRoot;

	return newSubRoot;
}

/*
* ��������
*/
static RBEntry* RotateRight(RBEntry* subRoot) {
	RBEntry* newSubRoot = subRoot->left;
	if (newSubRoot == NULL) {
		return subRoot;
	}

	newSubRoot->parent = subRoot->parent;
	if (subRoot->parent) {
		if (subRoot->parent->left == subRoot) {
			subRoot->parent->left = newSubRoot;
		}
		else {
			subRoot->parent->right = newSubRoot;
		}
	}
	subRoot->parent = newSubRoot;

	subRoot->left = newSubRoot->right;
	if (subRoot->left) {
		subRoot->left->parent = subRoot;
	}

	newSubRoot->right = subRoot;

	return newSubRoot;

}

static RBEntry* GetSiblingEntry(RBEntry* entry) {
	if (entry->parent->left == entry) {
		return entry->parent->right;
	}
	return entry->parent->left;
}

/*
* newEntry�ҽӵ�entryԭ����λ��
* entry������ժ������entry��parent��left��right����
*/
static void RBHitchEntry(RBTree* tree, RBEntry* entry, RBEntry* newEntry) {
	if (entry->parent) {
		if (entry->parent->left == entry) {
			entry->parent->left = newEntry;
		}
		else {
			entry->parent->right = newEntry;
		}
	}
	if (newEntry) {
		newEntry->parent = entry->parent;
	}
	if (tree->root == entry) {
		tree->root = newEntry;
	}
}

/*
* ���������ڵ㣬����parent��left��right��color���Լ��ڵ��parent��left��rightָ��ڵ��ָ��
* ���ܳ���������ʹ��(����entry2�ĸ��ڵ���entry1��������entry1�ĸ��ڵ��ָ���Լ���)
*/
static void RBSwapEntry(RBTree* tree, RBEntry* entry1, RBEntry* entry2) {
	MemorySwap(entry1, entry2, sizeof(RBEntry));
	if (entry1->parent) {
		if (entry1->parent->left == entry2) {
			entry1->parent->left = entry1;
		}
		else {
			entry1->parent->right = entry1;
		}
	}
	if (entry1->left) {
		entry1->left->parent = entry1;
	}
	if (entry1->right) {
		entry1->right->parent = entry1;
	}

	
	if (entry2->parent) {
		if (entry2->parent->left == entry1) {
			entry2->parent->left = entry2;
		}
		else {
			entry2->parent->right = entry2;
		}
	}
	if (entry2->left) {
		entry2->left->parent = entry2;
	}
	if (entry2->right) {
		entry2->right->parent = entry2;
	}

	if (tree->root == entry1) {
		tree->root = entry2;
	} else if (tree->root == entry2) {
		tree->root = entry1;
	}
}

#define RBTreeInitM(tree, objName, entryFieldName, keyFieldName) RBTreeInit((tree), sizeof(objName), GetFieldOffset(objName, entryFieldName), GetFieldOffset(objName, keyFieldName), GetFieldSize(objName, keyFieldName))


/*
* ��ʼ����
*/
void RBTreeInit(RBTree* tree, int objSize, int entryFieldOffset, int keyFieldOffset, int keySize) {
	tree->root = NULL;
	tree->entryFieldOffset = entryFieldOffset;
	// head->smallByteOrder = true;
	tree->keyFieldOffset = keyFieldOffset;
	tree->keyFieldSize = keySize;
	tree->objSize = objSize;
}

/*
* ��ʼ���ڵ�
*/
void RBEntryInit(RBEntry* entry, RBColor color) {
	entry->left = NULL;
	entry->right = NULL;
	entry->parent = NULL;
	entry->color = color;
}


/*
*/

/*
* �����в���ڵ�
* ����������ظ��ڵ�
* �ɹ�����true��ʧ�ܷ���false
*/
bool RBInsertEntry(RBTree* tree, RBEntry* entry) {
	RBEntry* root = tree->root;
	if (root == NULL) {
		RBEntryInit(entry, kBlack);
		tree->root = entry;
		return true;
	}
	RBEntryInit(entry, kRed);
	void* obj = GetObjByFieldOffset(entry, tree->entryFieldOffset, void);
	void* key = GetFieldByFieldOffset(obj, tree->keyFieldOffset, void);
	RBEntry* cur = root;
	while (cur) {
		void* curObj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
		int res = MemoryCmpR(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
		if (res < 0) {
			if (!cur->right) {
				cur->right = entry;
				break;
			}
			cur = cur->right;
		}
		else if (res > 0) {
			if (!cur->left) {
				cur->left = entry;
				break;
			}
			cur = cur->left;
		}
		else {
			return false;
		}
	}
	entry->parent = cur;
	if (cur && cur->color == kBlack) {
		// ��ǰ�ڵ�(����ڵ�ĸ��ڵ�)�Ǻ�ɫ��ɶ��������(��2�ڵ�/3�ڵ�Ĳ��룬ֱ�Ӻϲ�)
		return true;
	}

	RBEntry* newSubRoot = NULL;
	// ��ʼ����ά��
	while (cur) {
		if (cur->parent == NULL) {
			// û�и��ڵ㣬���ݵ����ڵ��ˣ�ֱ��Ⱦ��
			cur->color = kBlack;
			break;
		}
		RBEntry* sibling = GetSiblingEntry(cur);

		if (sibling && sibling->color == kRed) {
			// �ֵܽڵ��Ǻ�ɫ��˵����4�ڵ�Ĳ��룬����(����������־��Ǳ�ɫ)�����ڵ����Ϻϲ�����������
			cur->color = kBlack;
			sibling->color = kBlack;
			cur->parent->color = kRed;
		} else {
			// û���ֵܽڵ�����ֵܽڵ��Ǻ�ɫ��˵����3�ڵ�Ĳ��룬���Ժϲ�������Ҫ��ת�����Ϊ3�ڵ�
			//         10b               5b    
			//      5r     20b  ->   !2r     10r    
			//  !2r                             20b
			RBEntry* oldSubRoot = cur->parent;
			if (cur->parent->left == cur) {
				if (cur->right == entry) {
					RotateLeft(cur);
				}
				newSubRoot = RotateRight(cur->parent);
			} else {
				if (cur->left == entry) {
					RotateRight(cur);
				}
				newSubRoot = RotateLeft(cur->parent);
			}
			cur->color = kBlack;
			oldSubRoot->color = kRed;

			if (tree->root == oldSubRoot) {		// ԭ�����������ڵ�������������ĸ��ڵ㣬���Ҫ������
				tree->root = newSubRoot;
			}

			break;		// ֻ�Ǻϲ���δ���ѣ�����û�иı���ɫ��������Ҫ����
		}
		cur = cur->parent;
	}
	return true;
}


/*
* ������ɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
* ʵ�ֽ�Ϊ���ѣ���ʱ����
*/
RBEntry* RBDeleteEntry(RBTree* tree, RBEntry* entry) {
	RBEntry* backtrack = entry;		// ͨ��������Ǵӱ�ɾ���ڵ�ĸ��ڵ㿪ʼ����
	RBEntry* sibling = NULL;
	if (entry->left == NULL && entry->right == NULL) {
		// û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
		RBHitchEntry(tree, entry, NULL);
	}
	else if (entry->left == NULL) {
		// �ҽ����ӽڵ�
		RBHitchEntry(tree, entry, entry->right);
	}
	else if (entry->right == NULL) {
		// �ҽ����ӽڵ�
		RBHitchEntry(tree, entry, entry->left);
	}
	else {
		// �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
		RBEntry* minEntry = entry->right;
		while (minEntry) {
			if (minEntry->left) {
				minEntry = minEntry->left;
			}
			else {
				break;
			}
		}
		MemorySwap(&minEntry->color, &entry->color, sizeof(RBColor));

		// ��С�ڵ�̳д�ɾ���ڵ������������Ϊ��С�ڵ�϶�û����ڵ㣬����ֱ�Ӹ�ֵ
		minEntry->left = entry->left;
		if (minEntry->left) {
			minEntry->left->parent = minEntry;
		}

		// ��С�ڵ�����Ǵ�ɾ���ڵ���ҽڵ�
		if (minEntry->parent != entry) {
			// ��С�ڵ�̳д�ɾ���ڵ��������
			minEntry->parent->left = minEntry->right;
			if (minEntry->right) {
				minEntry->right->parent = minEntry->parent;
			}
			minEntry->right = entry->right;
			if (minEntry->right) {
				minEntry->right->parent = minEntry;
			}
		}

		RBEntry* oldParent = minEntry->parent;		// ��ɾ���Ľڵ㿪ʼ���ݣ������Ҫ��¼
		
		// �����йҽ�
		RBHitchEntry(tree, entry, minEntry);

		entry->parent = oldParent;		// ������Ҫ���ڵ��γɻ���·��

		sibling = 
	}
	if (entry) {
		if (entry->color == kRed) {
			// �Ǻ�ɫ�ģ���3/4�ڵ㣬��Ϊ��ʱһ����Ҷ�ӽڵ�(��ڵ㲻����ֻ��һ���ӽڵ�)��ֱ���Ƴ�
			return entry;
		}
		// �Ǻ�ɫ�ģ�������һ���ӽڵ㣬˵����3�ڵ㣬��Ϊ2�ڵ㼴��
		else if (entry->left) {
			entry->left->color = kBlack;
			return entry;
		}
		else if (entry->right) {
			entry->right->color = kBlack;
			return entry;
		}
	}


	// ����ά���Ǻ�ɫ�ģ�û���ӽڵ�(��2�ڵ�)�����
	while (backtrack) {
		if (!sibling) {
			sibling = GetSiblingEntry(backtrack);
		}
		sibling = NULL;
		backtrack = backtrack->parent;
	}
	return entry;
}


#ifdef __cplusplus
}
#endif

#endif // CUTILS_RBTREE_H_