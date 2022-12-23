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
static bool RotateFromBalanceFactor(AVLHead* head, AVLEntry** subRoot_) {
	AVLEntry* subRoot = *subRoot_;

	int factor = GetBalanceFactor(subRoot);
	bool rotate = false;
	AVLEntry* newSubRoot = NULL;
	if (factor > 1) {
		// ��ʧ��ڵ㣬�������߶ȸ����������߶�

		// �ж���Ҫ��������˫��
		if (GetBalanceFactor(subRoot->left) > 0) {
			// ʧ��ڵ���������������������������
			newSubRoot = RotateRight(subRoot);
		} else {
			// ʧ��ڵ��������������������ȶ�ʧ��ڵ���������������ٶ�ʧ��ڵ�����
			RotateLeft(subRoot->left);
			// ����ʧ��ڵ�����ڵ㽻��λ�ã���Ҫ�������������ʧ��ڵ��Ǹ��ڵ��ٷ����µĸ��ڵ�
			newSubRoot = RotateRight(subRoot);
		}
		rotate = true;
	} else if (factor < -1) {
		// ��ʧ��ڵ㣬�������߶ȸ����������߶�

		// �ж���Ҫ��������˫��
		if (GetBalanceFactor(subRoot->right) < 0) {
			// ʧ��ڵ���������������������������
			newSubRoot = RotateLeft(subRoot);
		} else {
			// ʧ��ڵ��������������������ȶ�ʧ��ڵ���������������ٶ�ʧ��ڵ�����
			RotateRight(subRoot->right);
			newSubRoot = RotateLeft(subRoot);
		}
		rotate = true;
	}

	if (rotate) {
		if (head->root == subRoot) {		// ԭ�����������ڵ�������������ĸ��ڵ㣬���Ҫ������
			head->root = newSubRoot;
		}
		*subRoot_ = newSubRoot;
	}

	return rotate;
}

/*
* newEntry�ҽӵ�entryԭ����λ��
* entry������ժ������entry��parent��left��right����
*/
static void AVLHitchEntry(AVLHead* head, AVLEntry* entry, AVLEntry* newEntry) {
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
	if (head->root == entry) {
		head->root = newEntry;
	}
}



void AVLHeadInit(AVLHead* head, int objSize, int entryOffset, int keyOffset, int keySize) {
	head->root = NULL;
	head->entryOffset = entryOffset;
	// head->smallByteOrder = true;
	head->keyOffset = keyOffset;
	head->keyByteCount = keySize;
	head->objByteCount = objSize;
}

void AVLEntryInit(AVLEntry* entry) {
	entry->height = 0;
	entry->left = NULL;
	entry->right = NULL;
	entry->parent = NULL;

}

AVLEntry* AVLFindKey(AVLHead* head, void* key) {
	AVLEntry* cur = head->root;
	void* obj = GetObjFromFieldOffset(void, cur, head->entryOffset);
	while (cur) {
		int res = MemoryCmpR(GetFieldFromObjOffset(void, obj, head->keyOffset), key, head->keyByteCount);
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

#define AVLFindKey_Macro()

bool AVLInsertEntry(AVLHead* head, AVLEntry* entry) {
	AVLEntry* root = head->root;
	if (root == NULL) {
		head->root = entry;
		return true;
	}

	void* obj = GetObjFromFieldOffset(void, entry, head->entryOffset);
	void* key = GetFieldFromObjOffset(void, obj, head->keyOffset);
	AVLEntry* cur = (AVLEntry*)root;
	while (cur) {
		void* curObj = GetObjFromFieldOffset(void, cur, head->entryOffset);
		int res = MemoryCmpR(GetFieldFromObjOffset(void, curObj, head->keyOffset), key, head->keyByteCount);
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
		if (RotateFromBalanceFactor(head, &cur)) {
			break;		// ����������ת�ˣ��Ͳ���Ҫ�����ϻ����ˣ���Ϊ��ת�ᵼ����������ĸ߶Ȳ���
		}
		cur = cur->parent;
		heightCount++;
	}
	return true;
}

AVLEntry* AVLDeleteEntry(AVLHead* head, void* key) {
	AVLEntry* cur = head->root;
	AVLEntry* backtrack = NULL;
	while (cur) {
		void* curObj = GetObjFromFieldOffset(void, cur, head->entryOffset);
		int res = MemoryCmpR(GetFieldFromObjOffset(void, curObj, head->keyOffset), key, head->keyByteCount);
		if (res < 0) {
			cur = cur->right;
		} else if (res > 0) {
			cur = cur->left;
		} else {
			// �ҵ��ڵ㣬׼��ɾ��
			if (cur->left == NULL && cur->right == NULL) {
				// û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
				AVLHitchEntry(head, cur, NULL);
			} else if (cur->left == NULL) {
				// ֻ�����ӽڵ㣬��˵�����ӽڵ�û���ӽڵ�(���ӽڵ�Ļ����Ѿ�ʧ���ˣ���Ϊû�����ӽڵ㣬���ӽڵ㻹���ӽڵ�ͻ��γ�0 - 2)
				AVLHitchEntry(head, cur, cur->right);
			} else if (cur->right == NULL) {
				AVLHitchEntry(head, cur, cur->left);
			} else {
				// �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
				AVLEntry* minEntry = cur->right;
				while (minEntry) {
					if (minEntry->left) {
						minEntry = minEntry->left;
					} else {
						break;
					}
				}
				
				// ��С�ڵ�̳д�ɾ���ڵ��������
				minEntry->left = cur->left;
				if (minEntry->left) {
					minEntry->left->parent = minEntry;
				}

				// ��С�ڵ�̳д�ɾ���ڵ��������
				if (minEntry->parent != cur) {
					backtrack = minEntry->parent;

					minEntry->parent->left = minEntry->right;
					if (minEntry->right) {
						minEntry->right->parent = minEntry->parent;
					}
					minEntry->right = cur->right;
					if (minEntry->right) {
						minEntry->right->parent = minEntry;
					}
				} else {
					backtrack = minEntry;
				}
				AVLHitchEntry(head, cur, minEntry);
				break;
			}
			backtrack = cur->parent;
			break;
		}
	}
	// ɾ���ڵ��߶ȿ��ܷ����仯������ά���ڵ�߶�
	while (backtrack) {
		if (UpdateHeight(backtrack) == false) {
			// ���ڵ�߶�δ�仯��˵�������ڵ����һ������ȸ��Ҫ����Ƿ�ʧ��
			if (RotateFromBalanceFactor(head, &backtrack) == false) {
				break;		// δʧ����ֹͣ���ݣ��ӵ�ǰ�ڵ㿪ʼ������Ӱ���ϲ�ڵ�ĸ߶ȡ�
			}
		} else { }
		backtrack = backtrack->parent;
	}
	return cur;
}

static void AVLEntrySizeCountCallback(AVLEntry* entry, void* arg) {
	int* count = arg;
	(*count)++;
}
size_t AVLEntrySize(AVLHead* head) {
	int count = 0;
	AVLPreorder_Callback(head->root, AVLEntrySizeCountCallback, &count);
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
void Middleorder_Callback(AVLEntry* entry, TraversalCallback callback, void* arg) {
	if (!entry) return;
	Middleorder_Callback(entry->left, callback, arg);
	callback(entry, arg);
	Middleorder_Callback(entry->right, callback, arg);
}

/*
* �������
* ���������ٸ�
*/
void Postorder_Callback(AVLEntry* entry, TraversalCallback callback, void* arg) {
	if (!entry) return;
	Postorder_Callback(entry->left, callback, arg);
	Postorder_Callback(entry->right, callback, arg);
	callback(entry, arg);
}


/*
* �������
* �����ٸ�����
*/
bool AVLMiddleorder(AVLEntry** cur, bool* status_right) {
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
