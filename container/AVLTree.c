#include "AVLTree.h"

/*
* �������������߶Ȼ�ȡ�ڵ�߶�
*/
static int GetMaxHeight(AVLEntry* subRoot) {
	int leftHeight = -1, rightHeight = -1;
	if (subRoot->left) leftHeight = subRoot->left->height;
	if (subRoot->right) rightHeight = subRoot->right->height;
	return max(leftHeight, rightHeight) + 1;
}

/*
* �������������߶ȸ��½ڵ�߶�
* ������ɷ���true��������·���false
*/
static int UpdateHeight(AVLEntry* subRoot) {
	int newHeight = GetMaxHeight(subRoot);
	if (subRoot->height != newHeight) {
		subRoot->height = newHeight;
		return true;
	}
	return false;
}

/*
* �������������߶Ȼ�ȡ�ڵ�ƽ������
*/
static int GetBalanceFactor(AVLEntry* subRoot) {
	int leftHeight = -1, rightHeight = -1;
	if (subRoot->left) leftHeight = subRoot->left->height;
	if (subRoot->right) rightHeight = subRoot->right->height;
	return leftHeight - rightHeight;
}

/*
* ��������
*/
static AVLEntry* RotateLeft(AVLEntry* subRoot) {
	AVLEntry* newSubRoot = subRoot->right;
	if (newSubRoot == NULL) {
		return subRoot;
	}

	newSubRoot->parent = subRoot->parent;
	if (subRoot->parent) {
		if (subRoot->parent->left == subRoot) {
			subRoot->parent->left = newSubRoot;
		} else {
			subRoot->parent->right = newSubRoot;
		}
	}
	subRoot->parent = newSubRoot;
	
	subRoot->right = newSubRoot->left;
	if (subRoot->right) {
		subRoot->right->parent = subRoot;
	}

	newSubRoot->left = subRoot;
		
	UpdateHeight(subRoot);
	UpdateHeight(newSubRoot);

	return newSubRoot;
}

/*
* ��������
*/
static AVLEntry* RotateRight(AVLEntry* subRoot) {
	AVLEntry* newSubRoot = subRoot->left;
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

	UpdateHeight(subRoot);
	UpdateHeight(newSubRoot);

	return newSubRoot;

}

/*
* ����ƽ����������ת����
*/
static bool RotateByBalanceFactor(AVLTree* tree, AVLEntry** subRoot_) {
	AVLEntry* subRoot = *subRoot_;

	int factor = GetBalanceFactor(subRoot);
	bool rotate = false;
	AVLEntry* newSubRoot = NULL;
	if (factor > 1) {
		// ��ʧ��ڵ㣬�������߶ȸ����������߶�

		// �ж���Ҫ��������˫��
		if (GetBalanceFactor(subRoot->left) < 0) {
			// ʧ��ڵ��������������������ȶ�ʧ��ڵ���������������ٶ�ʧ��ڵ�����
			RotateLeft(subRoot->left);
		}
		// ��ʱʧ��ڵ���������������������������
		// ����ʧ��ڵ�����ڵ㽻��λ�ã���Ҫ�������������ʧ��ڵ��Ǹ��ڵ��ٷ����µĸ��ڵ�
		newSubRoot = RotateRight(subRoot);
		rotate = true;
	} else if (factor < -1) {
		// ��ʧ��ڵ㣬�������߶ȸ����������߶�

		// �ж���Ҫ��������˫��
		if (GetBalanceFactor(subRoot->right) > 0) {
			// ʧ��ڵ��������������������ȶ�ʧ��ڵ���������������ٶ�ʧ��ڵ�����
			RotateRight(subRoot->right);
		}
		// ��ʱʧ��ڵ���������������������������
		newSubRoot = RotateLeft(subRoot);

		rotate = true;
	}

	if (rotate) {
		if (tree->root == subRoot) {		// ԭ�����������ڵ�������������ĸ��ڵ㣬���Ҫ������
			tree->root = newSubRoot;
		}
		*subRoot_ = newSubRoot;
	}

	return rotate;
}

/*
* newEntry�ҽӵ�entryԭ����λ��
* entry������ժ������entry��parent��left��right����
*/
static void AVLHitchEntry(AVLTree* tree, AVLEntry* entry, AVLEntry* newEntry) {
	if (entry->parent) {
		if (entry->parent->left == entry) {
			entry->parent->left = newEntry;
		} else {
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
void AVLTreeInit(AVLTree* tree, int objSize, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
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
void AVLEntryInit(AVLEntry* entry) {
	entry->height = 0;
	entry->left = NULL;
	entry->right = NULL;
	entry->parent = NULL;
}

/*
* �����в��ҽڵ�
* ���ڷ��ز��ҵ��Ľڵ㣬�����ڷ���NULL
*/
AVLEntry* AVLFindEntryByKey(AVLTree* tree, void* key) {
	AVLEntry* cur = tree->root;
	while (cur) {
		void* obj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
		int res = tree->cmpFunc(GetFieldByFieldOffset(obj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
		if (res < 0) {
			cur = cur->right;
		} else if (res > 0) {
			cur = cur->left;
		} else {
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
bool AVLInsertEntry(AVLTree* tree, AVLEntry* entry) {
	AVLEntry* root = tree->root;
	AVLEntryInit(entry);
	if (root == NULL) {
		tree->root = entry;
		return true;
	}

	void* obj = GetObjByFieldOffset(entry, tree->entryFieldOffset, void);
	void* key = GetFieldByFieldOffset(obj, tree->keyFieldOffset, void);
	AVLEntry* cur = root;
	while (cur) {
		void* curObj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
		int res = tree->cmpFunc(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
		if (res < 0) {
			if (!cur->right) {
				cur->right = entry;
				break;
			}
			cur = cur->right;
		} else if (res > 0) {
			if (!cur->left) {
				cur->left = entry;
				break;
			}
			cur = cur->left;
		} else {
			return false;
		}
	}
	entry->parent = cur;

	// ����ڵ��߶ȿ��ܷ����仯������ά���ڵ�߶�
	int heightCount = 1;
	while (cur) {
		if (cur->height < heightCount) {
			cur->height = heightCount;
		} else {
			break;		// ���˸߶�δ�����仯�����ٻ���
		}
		if (RotateByBalanceFactor(tree, &cur)) {
			break;		// ����������ת�ˣ��Ͳ���Ҫ�����ϻ����ˣ���Ϊ��ת�ᵼ����������ĸ߶Ȳ���
		}
		cur = cur->parent;
		heightCount++;
	}
	return true;
}

/*
* ������ɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
*/
AVLEntry* AVLDeleteEntry(AVLTree* tree, AVLEntry* entry) {

	AVLEntry* backtrack = entry->parent;		// ͨ��������Ǵӱ�ɾ���ڵ�ĸ��ڵ㿪ʼ����
	if (entry->left == NULL && entry->right == NULL) {
		// û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
		AVLHitchEntry(tree, entry, NULL);
	}
	else if (entry->left == NULL) {
		// ֻ�����ӽڵ㣬��˵�����ӽڵ�û���ӽڵ�(���ӽڵ�Ļ����Ѿ�ʧ���ˣ���Ϊû�����ӽڵ㣬���ӽڵ㻹���ӽڵ�ͻ��γ�0 - 2)
		AVLHitchEntry(tree, entry, entry->right);
	}
	else if (entry->right == NULL) {
		AVLHitchEntry(tree, entry, entry->left);
	}
	else {
		// �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
		AVLEntry* minEntry = entry->right;
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
			backtrack = minEntry->parent;		// ���޸���С�ڵ�ĸ��ڵ�֮ǰ��¼���ݽڵ�

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
		else {
			backtrack = minEntry;		// ��С�ڵ�ĸ��׾��Ǵ�ɾ���ڵ㣬����λ�ú���С�ڵ���Ǵ�ɾ���ڵ�ĸ��ף���˴��������
		}

		// �����йҽ�
		AVLHitchEntry(tree, entry, minEntry);

		// Ҳ����ѡ��ֱ�ӽ��������ڵ������
	}
	
	
	// ɾ���ڵ��߶ȿ��ܷ����仯������ά���ڵ�߶�
	while (backtrack) {
		if (UpdateHeight(backtrack) == false) {
			// ���ڵ�߶�δ�仯��˵�������ڵ����һ������ȸ��Ҫ����Ƿ�ʧ��
			if (RotateByBalanceFactor(tree, &backtrack) == false) {
				break;		// δʧ����ֹͣ���ݣ��ӵ�ǰ�ڵ㿪ʼ������Ӱ���ϲ�ڵ�ĸ߶ȡ�
			}
		}
		else {}
		backtrack = backtrack->parent;
	}
	return entry;
}

/*
* �����а�keyɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
*/
AVLEntry* AVLDeleteEntryByKey(AVLTree* tree, void* key) {
	AVLEntry* entry = AVLFindEntryByKey(tree, key);
	if (entry) {
		AVLDeleteEntry(tree, entry);
	}
	return entry;
}

/*
* ��ȡ���ڵ�����
*/
static void AVLGetEntryCountCallback(AVLEntry* entry, void* arg) {
	int* count = arg;
	(*count)++;
}
size_t AVLGetEntryCount(AVLTree* head) {
	int count = 0;
	AVLPreorder_Callback(head->root, AVLGetEntryCountCallback, &count);
	return count;
}



/*
* ǰ�����
* �ȸ���������
*/
void AVLPreorder_Callback(AVLEntry* entry, TraversalCallback callback, void* arg) {
	if (!entry) return;
	callback(entry, arg);
	AVLPreorder_Callback(entry->left, callback, arg);
	AVLPreorder_Callback(entry->right, callback, arg);
}

/*
* �������
* �����ٸ�����
*/
void AVLMiddleorder_Callback(AVLEntry* entry, TraversalCallback callback, void* arg) {
	if (!entry) return;
	AVLMiddleorder_Callback(entry->left, callback, arg);
	callback(entry, arg);
	AVLMiddleorder_Callback(entry->right, callback, arg);
}

/*
* �������
* ���������ٸ�
*/
void AVLPostorder_Callback(AVLEntry* entry, TraversalCallback callback, void* arg) {
	if (!entry) return;
	AVLPostorder_Callback(entry->left, callback, arg);
	AVLPostorder_Callback(entry->right, callback, arg);
	callback(entry, arg);
}


/*
* �������
* �����ٸ�����
*/
bool AVLMiddleorder_Iteration(AVLEntry** cur, bool* status_right) {
	if (*status_right == true) {
		// ��ǰ�ڵ���������Ѿ��������ˣ�׼������������ʼ
		if ((*cur)->right == NULL) {
			// û������������ǰ�����Ѿ��������ˣ������ҵ�һ��ѭ���ڵ����丸�ڵ����ڵ�Ľڵ�(���������ҵ���һ����������ȫ��������ϵ��������ڵ�)
			while ((*cur)->parent) {
				if (*cur == (*cur)->parent->left) {
					*cur = (*cur)->parent;
					break;
				}
				*cur = (*cur)->parent;
			};
			
			return *cur ? true : false;		// ���û�и��ڵ���˵���������������
		}
		// �����������������������
		*cur = (*cur)->right;
	}

	if ((*cur)->left) {
		// �����������ҵ�����Ľڵ㽫�䷵�أ�ͬʱ������������������ڵ�(��Ϊ���صĽڵ�û�������������������������ڵ�)�Ѿ�����
		do {
			(*cur) = (*cur)->left;
		} while((*cur)->left);
		*status_right = true;
		return true;
	}

	// û�������������ظýڵ㲢������������������ڵ��Ѿ�����
	*status_right = true;
	return true;
}

/*
* �������
* ���������ٸ�
*/
AVLEntry* AVLPostorder(AVLEntry* cur) {
	//if (cur->left || cur->right) {
	//	while (cur->left) {
	//		cur = cur->left;
	//		if (cur->left) {
	//			continue;
	//		}
	//		while (cur->right) {
	//			cur = cur->right;
	//		};
	//		
	//	};
	//	
	//	return cur;
	//}

	//if (cur->parent == NULL) {
	//	return cur;
	//}
	//if (cur == cur->parent->left) {
	//	// �Ǹ��ڵ����ڵ㣬���ڵ��������һ����û�б���
	//	return cur->parent->right;
	//}

	//// �Ǹ��ڵ���ҽڵ㣬��Ҫ�����ҵ���һ�������ڵ����丸�ڵ����ڵ�ı����ڵ㸸�ڵ㣬������������
	//cur = cur->parent;
	//if (cur->parent == NULL) {
	//	// ���ڵ㼴���ڵ㣬ֱ�ӷ��ظ��ڵ㣬�����һ�˱���
	//	return cur->parent;
	//}
	//while (cur->parent->left != cur->parent) {
	//	cur = cur->parent;
	//}
	//return cur;
	
}
