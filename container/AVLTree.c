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
* ��ʼ����
*/
void AVLTreeInit(AVLTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
	BSTreeInit((BSTree*)tree, entryFieldOffset, keyFieldOffset, keySize, cmpFunc);
}

/*
* ��ʼ���ڵ�
*/
void AVLEntryInit(AVLEntry* entry) {
	BSEntryInit(&entry->bs);
	entry->height = 0;
}

/*
* �����в��ҽڵ�
* ���ڷ��ز��ҵ��Ľڵ㣬�����ڷ���NULL
*/
AVLEntry* AVLFindEntryByKey(AVLTree* tree, void* key) {
	return (AVLEntry*)BSFindEntryByKey((BSTree*)tree, key);
}

/*
* �����в���ڵ�
* ����������ظ��ڵ�
* �ɹ�����true��ʧ�ܷ���false
*/
bool AVLInsertEntry(AVLTree* tree, AVLEntry* entry) {
	if (!BSInsertEntry((BSTree*)tree, &entry->bs)) {
		return false;
	}
	entry->height = 0;
	AVLEntry* cur = entry->parent;

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
	AVLEntry* backtrack = (AVLEntry*)BSDeleteEntry(tree, entry);
	if (backtrack) {
		backtrack = backtrack->parent;
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
	BSPreorder_Callback(head->root, AVLGetEntryCountCallback, &count);
	return count;
}