#include "BSTree.h"


/*
* newEntry�ҽӵ�entryԭ����λ��
* entry������ժ������entry��parent��left��right����
*/
static void BSHitchEntry(BSTree* tree, BSEntry* entry, BSEntry* newEntry) {
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
* ��ʼ����
*/
void BSTreeInit(BSTree* tree, int objSize, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
	tree->root = NULL;
	tree->entryFieldOffset = entryFieldOffset;
	// head->smallByteOrder = true;
	tree->keyFieldOffset = keyFieldOffset;
	tree->keyFieldSize = keySize;
	tree->objSize = objSize;
	if (cmpFunc == NULL) {
		cmpFunc = MemoryCmpR;
	}
	tree->cmpFunc = cmpFunc;
}

/*
* ��ʼ���ڵ�
*/
void BSEntryInit(BSEntry* entry) {
	entry->left = NULL;
	entry->right = NULL;
	entry->parent = NULL;
}

/*
* �����в��ҽڵ�
* ���ڷ��ز��ҵ��Ľڵ㣬�����ڷ���NULL
*/
BSEntry* BSFindEntryByKey(BSTree* tree, void* key) {
	BSEntry* cur = tree->root;
	while (cur) {
		void* obj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
		int res = tree->cmpFunc(GetFieldByFieldOffset(obj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
		if (res < 0) {
			cur = cur->right;
		}
		else if (res > 0) {
			cur = cur->left;
		}
		else {
			return cur;
		}
	}
	return NULL;
}

/*
* �����в���ڵ�
* ����������ظ��ڵ�
* �ɹ�����true��ʧ�ܷ���false
*/
bool BSInsertEntry(BSTree* tree, BSEntry* entry) {
	BSEntry* root = tree->root;
	BSEntryInit(entry);
	if (root == NULL) {
		tree->root = entry;
		return true;
	}
	void* obj = GetObjByFieldOffset(entry, tree->entryFieldOffset, void);
	void* key = GetFieldByFieldOffset(obj, tree->keyFieldOffset, void);
	BSEntry* cur = root;
	while (cur) {
		void* curObj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
		int res = tree->cmpFunc(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
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
	return true;
}

/*
* ������ɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
*/
BSEntry* BSDeleteEntry(BSTree* tree, BSEntry* entry) {
	if (entry->left == NULL && entry->right == NULL) {
		// û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
		BSHitchEntry(tree, entry, NULL);
	}
	else if (entry->left == NULL) {
		// ֻ�����ӽڵ㣬��˵�����ӽڵ�û���ӽڵ�(���ӽڵ�Ļ����Ѿ�ʧ���ˣ���Ϊû�����ӽڵ㣬���ӽڵ㻹���ӽڵ�ͻ��γ�0 - 2)
		BSHitchEntry(tree, entry, entry->right);
	}
	else if (entry->right == NULL) {
		BSHitchEntry(tree, entry, entry->left);
	}
	else {
		// �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
		BSEntry* minEntry = entry->right;
		while (minEntry) {
			if (minEntry->left) {
				minEntry = minEntry->left;
			}
			else {
				break;
			}
		}

		// ��С�ڵ�̳д�ɾ���ڵ������������Ϊ��С�ڵ�϶�û����ڵ㣬����ֱ�Ӹ�ֵ
		minEntry->left = entry->left;
		if (entry->left) {
			entry->left->parent = minEntry;
		}

		// ��С�ڵ�����Ǵ�ɾ���ڵ���ҽڵ�
		if (minEntry->parent != entry) {
			// ��minEntry��ԭ�ȵ�λ��ժ������������������
			minEntry->parent->left = minEntry->right;
			if (minEntry->right) {
				minEntry->right->parent = minEntry->parent;
			}
			// ��С�ڵ�̳д�ɾ���ڵ��������
			minEntry->right = entry->right;
			if (entry->right) {
				entry->right->parent = minEntry;
			}
		}

		// �����йҽ�
		BSHitchEntry(tree, entry, minEntry);

		// Ҳ����ѡ��ֱ�ӽ��������ڵ������
	}
	return entry;
}


BSEntry* BSFirst(BSTree* tree) {
	BSEntry* cur = tree->root;
	if (!cur) {
		return NULL;
	}
	while (cur->left)
		cur = cur->left;
	return cur;
}

BSEntry* BSLast(BSTree* tree) {
	BSEntry* cur = tree->root;
	if (!cur) {
		return NULL;
	}
	while (cur->right)
		cur = cur->right;
	return cur;
}

BSEntry* BSNext(BSEntry* entry) {
	if (entry->right) {
		entry = entry->right;
		while (entry->left)
			entry = entry->left;
		return entry;
	}
	BSEntry* parent;
	while ((parent = entry->parent) && entry == parent->right)
		entry = parent;
	return parent;
}

BSEntry* BSPrev(BSEntry* entry) {
	if (entry->left) {
		entry = entry->left;
		while (entry->right) {
			entry = entry->right;
		}
		return entry;
	}
	BSEntry* parent;
	while ((parent = entry->parent) && entry == parent->left) {
		entry = parent;
	}
	return parent;
}



/*
* ǰ�����
* �ȸ���������
*/
void BSPreorder_Callback(BSEntry* entry, BSTraversalCallback callback, void* arg) {
	if (!entry) return;
	callback(entry, arg);
	BSPreorder_Callback(entry->left, callback, arg);
	BSPreorder_Callback(entry->right, callback, arg);
}

/*
* �������
* �����ٸ�����
*/
void BSMiddleorder_Callback(BSEntry* entry, BSTraversalCallback callback, void* arg) {
	if (!entry) return;
	BSMiddleorder_Callback(entry->left, callback, arg);
	callback(entry, arg);
	BSMiddleorder_Callback(entry->right, callback, arg);
}

/*
* �������
* ���������ٸ�
*/
void BSPostorder_Callback(BSEntry* entry, BSTraversalCallback callback, void* arg) {
	if (!entry) return;
	BSPostorder_Callback(entry->left, callback, arg);
	BSPostorder_Callback(entry->right, callback, arg);
	callback(entry, arg);
}