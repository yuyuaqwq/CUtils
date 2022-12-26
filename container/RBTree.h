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
	int entryFieldOffset;
	int objByteCount;
	int keyFieldOffset;
	int keyByteCount;
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


#define RBTreeInitM(tree, objName, entryFieldName, keyFieldName) RBTreeInit((tree), sizeof(objName), GetFieldOffset(objName, entryFieldName), GetFieldOffset(objName, keyFieldName), GetFieldSize(objName, keyFieldName))


/*
* ��ʼ����
*/
void RBTreeInit(RBTree* tree, int objSize, int entryFieldOffset, int keyFieldOffset, int keySize) {
	tree->root = NULL;
	tree->entryFieldOffset = entryFieldOffset;
	// head->smallByteOrder = true;
	tree->keyFieldOffset = keyFieldOffset;
	tree->keyByteCount = keySize;
	tree->objByteCount = objSize;
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

static RBEntry* GetGrandfatherEntry(RBEntry* entry) {
	return entry->parent->parent;
}

static RBEntry* GetSiblingEntry(RBEntry* entry) {
	if (entry->parent->left == entry) {
		return entry->parent->right;
	}
	return entry->parent->left;
}


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
		int res = MemoryCmpR(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyByteCount);
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

	RBEntry* newSubRoot = NULL;
	// ��ʼ����ά��
	while (cur) {
		if (cur->color == kBlack) {
			// ��ǰ�ڵ�(����ڵ�ĸ��ڵ�)�Ǻ�ɫ��ɶ��������(��2�ڵ�/3�ڵ�Ĳ��룬�ϲ�)
			break;
		}
		// ��ǰ�Ǻ�ڵ�
		if (cur->parent == NULL) {
			// û�и��ڵ㣬���ݵ����ڵ��ˣ�ֱ��Ⱦ��
			cur->color = kBlack;
			break;
		}
		RBEntry* sibling = GetSiblingEntry(cur);

		if (sibling && sibling->color == kRed) {
			// �ֵܽڵ��Ǻ�ɫ��˵����4�ڵ�Ĳ��룬���ѣ����ڵ����Ϻϲ�����������
			cur->color = kBlack;
			sibling->color = kBlack;
			cur->parent->color = kRed;
		} else {
			// û���ֵܽڵ�����ֵܽڵ��Ǻ�ɫ��˵����3�ڵ�Ĳ��룬���Ժϲ�������Ҫ��ת
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


#ifdef __cplusplus
}
#endif

#endif // CUTILS_RBTREE_H_