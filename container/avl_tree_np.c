#include <CUtils/container/avl_tree_np.h>

/*
* ��ȡ���ӽڵ�
*/
inline AVLEntry* AVLEntryGetLeft(AVLEntry* entry) {
    return (AVLEntry*)(((uintptr_t)entry->left_balanceFactor) & (~((uintptr_t)0x3)));
}

/*
* �������ӽڵ�
*/
inline void AVLEntrySetLeft(AVLEntry* entry, AVLEntry* left) {
    entry->left_balanceFactor = (AVLEntry*)(((uintptr_t)left) | ((uintptr_t)entry->left_balanceFactor & 0x3));
}

/*
* ��ȡ���ӽڵ�
*/
inline AVLEntry* AVLEntryGetRight(AVLEntry* entry) {
    return entry->right;
}

/*
* �������ӽڵ�
*/
inline void AVLEntrySetRight(AVLEntry* entry, AVLEntry* right) {
    entry->right = right;
}

/*
* ��ȡ�ڵ�ƽ������
*/
inline int AVLEntryGetBalanceFactor(AVLEntry* entry) {
    int ret = (int)(((uintptr_t)entry->left_balanceFactor) & 0x3);
    if (ret == 3) {
        ret = -1;
    }
    return ret;
}

/*
* ���ýڵ�ƽ������
*/
inline void AVLEntrySetBalanceFactor(AVLEntry* entry, int balanceFactor) {
    entry->left_balanceFactor = (AVLEntry*)(((uintptr_t)AVLEntryGetLeft(entry)) | ((uintptr_t)balanceFactor) & 0x3);
}


/*
* ��������
*/
static AVLEntry* RotateLeft(AVLEntry* subRoot, AVLEntry* subRootParent) {
    AVLEntry* newSubRoot = AVLEntryGetRight(subRoot);
    if (newSubRoot == NULL) {
        return subRoot;
    }
    if (subRootParent) {
        if (AVLEntryGetLeft(subRootParent) == subRoot) {
            AVLEntrySetLeft(subRootParent, newSubRoot);
        }
        else {
            AVLEntrySetRight(subRootParent, newSubRoot);
        }
    }
    AVLEntrySetRight(subRoot, AVLEntryGetLeft(newSubRoot));
    AVLEntrySetLeft(newSubRoot, subRoot);
    return newSubRoot;
}

/*
* ��������
*/
static AVLEntry* RotateRight(AVLEntry* subRoot, AVLEntry* subRootParent) {
    AVLEntry* newSubRoot = AVLEntryGetLeft(subRoot);
    if (newSubRoot == NULL) {
        return subRoot;
    }
    if (subRootParent) {
        if (AVLEntryGetLeft(subRootParent) == subRoot) {
            AVLEntrySetLeft(subRootParent, newSubRoot);
        }
        else {
            AVLEntrySetRight(subRootParent, newSubRoot);
        }
    }
    AVLEntrySetLeft(subRoot, AVLEntryGetRight(newSubRoot));
    AVLEntrySetRight(newSubRoot, subRoot);
    return newSubRoot;

}

/*
* newEntry����entry�ҽӵ��丸�ڵ���
* newEntry�������ӽڵ㲻��
* entry������ժ��
* entry��left��right����
*/
static void AVLTreeHitchEntry(AVLTree* tree, AVLEntry* entry, AVLEntry* entryParent, AVLEntry* newEntry) {
    if (entryParent) {
        if (AVLEntryGetLeft(entryParent) == entry) {
            AVLEntrySetLeft(entryParent, newEntry);
        }
        else {
            entryParent->right = newEntry;
        }
    }
    if (tree->root == entry) {
        tree->root = newEntry;
    }
}


/*
* ����ƽ����������ת����
* ���������ƽ������
* �����߶ȱ仯����true���߶�δ�仯����false
*/
static bool RotateByBalanceFactor(AVLTree* tree, AVLEntry** unbalancedEntry, AVLEntry* subRoot, int curBF, AVLEntry* parent) {
    bool rotate = false;
    bool heightUpdate = true;
    AVLEntry* newSubRoot = NULL;
    if (curBF > 1) {
        AVLEntry* child = AVLEntryGetLeft(subRoot);
        int childBF = AVLEntryGetBalanceFactor(child);
        if (childBF == -1) {
            int rightBF = AVLEntryGetBalanceFactor(child->right);
            if (child->right) {
                AVLEntrySetBalanceFactor(child->right, 0);
            }
            RotateLeft(child, subRoot);
            newSubRoot = RotateRight(subRoot, parent);

            if (rightBF == -1) {
                AVLEntrySetBalanceFactor(child, 1);
                AVLEntrySetBalanceFactor(subRoot, 0);
            }
            else {
                AVLEntrySetBalanceFactor(child, 0);
                if (rightBF == 1) {
                    AVLEntrySetBalanceFactor(subRoot, -1);
                }
                else {
                    AVLEntrySetBalanceFactor(subRoot, 0);
                }
            }
        }
        else if (childBF == 0) {
            heightUpdate = false;
            newSubRoot = RotateRight(subRoot, parent);
            AVLEntrySetBalanceFactor(child, -1);
            AVLEntrySetBalanceFactor(subRoot, 1);
        }
        else {
            newSubRoot = RotateRight(subRoot, parent);
            AVLEntrySetBalanceFactor(child, 0);
            AVLEntrySetBalanceFactor(subRoot, 0);
        }
        rotate = true;
    }
    else if (curBF < -1) {
        AVLEntry* child = subRoot->right;
        int childBF = AVLEntryGetBalanceFactor(child);
        if (childBF == 1) {
            int leftBF = AVLEntryGetBalanceFactor(AVLEntryGetLeft(child));
            if (AVLEntryGetLeft(child)) {
                AVLEntrySetBalanceFactor(AVLEntryGetLeft(child), 0);
            }
            RotateRight(child, subRoot);
            newSubRoot = RotateLeft(subRoot, parent);
            if (leftBF == 1) {
                AVLEntrySetBalanceFactor(child, -1);
                AVLEntrySetBalanceFactor(subRoot, 0);
            }
            else {
                AVLEntrySetBalanceFactor(child, 0);
                if (leftBF == -1) {
                    AVLEntrySetBalanceFactor(subRoot, 1);
                }
                else {
                    AVLEntrySetBalanceFactor(subRoot, 0);
                }
            }
        }
        else if (childBF == 0) {
            heightUpdate = false;
            newSubRoot = RotateLeft(subRoot, parent);
            AVLEntrySetBalanceFactor(child, 1);
            AVLEntrySetBalanceFactor(subRoot, -1);
        }
        else {
            newSubRoot = RotateLeft(subRoot, parent);
            AVLEntrySetBalanceFactor(child, 0);
            AVLEntrySetBalanceFactor(subRoot, 0);
        }
        rotate = true;
    }
    else {
        heightUpdate = false;
        if (unbalancedEntry) *unbalancedEntry = subRoot;
        AVLEntrySetBalanceFactor(subRoot, curBF);
    }
    if (rotate) {
        if (tree->root == subRoot) {
            tree->root = newSubRoot;
        }
    }
    return heightUpdate;
}


/*
* ��ʼ��AVL��
*/
void AVLTreeInit(AVLTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
    BSTreeInit(&tree->bst, entryFieldOffset, keyFieldOffset, keySize, cmpFunc);
}

/*
* ��ʼ���ڵ�
*/
void AVLEntryInit(AVLEntry* entry) {
    BSEntryInit(&entry->bse);
    AVLEntrySetBalanceFactor(entry, 0);
}




/*
* �����в���ڵ���ƽ�����
* ����ƽ�ⷵ��true������ƽ�ⷵ��false
*/
bool AVLTreeInsertEntryFixup(AVLTree* tree, AVLEntry** unbalancedEntry, AVLEntry* cur, AVLEntry* curParent) {
    // ����ڵ��ƽ�����ӿ��ܷ����仯������ά��ƽ������
    int curBF = AVLEntryGetBalanceFactor(cur);
    if (AVLEntryGetLeft(cur) == *unbalancedEntry) curBF++;        // �½ڵ���뵽��ǰ�ڵ��������
    else curBF--;       // �½ڵ���뵽��ǰ�ڵ��������

    if (RotateByBalanceFactor(tree, unbalancedEntry, cur, curBF, curParent) || curBF == 0) {
        // ��ת��ǰ�ڵ�߶Ȳ��䣬��ԭ�ȸ߶ȾͲ��䣬ֹͣ����
        return false;
    }
    return true;
}

/*
* �ݹ���Ҳ�����ڵ�
*/
bool AVLTreeInsertEntry(AVLTree* tree, AVLEntry** unbalancedEntry, AVLEntry* cur, AVLEntry* curParent) {
    void* obj = GetObjByFieldOffset(*unbalancedEntry, tree->entryFieldOffset, void);
    void* key = GetFieldByFieldOffset(obj, tree->keyFieldOffset, void);

    void* curObj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
    int res = tree->cmpFunc(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
    bool ret = true;
    if (res == 0) {
        return false;
    }
    else if (res < 0) {
        if (AVLEntryGetRight(cur)) {
            ret = AVLTreeInsertEntry(tree, unbalancedEntry, AVLEntryGetRight(cur), cur);
        }
        else {
            AVLEntrySetRight(cur, *unbalancedEntry);
        }
    }
    else {
        if (AVLEntryGetLeft(cur)) {
            ret = AVLTreeInsertEntry(tree, unbalancedEntry, AVLEntryGetLeft(cur), cur);
        }
        else {
            AVLEntrySetLeft(cur, *unbalancedEntry);
        }
    }
    if (ret) {
        ret = AVLTreeInsertEntryFixup(tree, unbalancedEntry, cur, curParent);
    }
    return ret;
}

/*
* �����а�key����ڵ�
* ����������ظ��ڵ�
* �ɹ�����true��ʧ�ܷ���false
*/
bool AVLTreeInsertEntryByKey(AVLTree* tree, AVLEntry* entry) {
    AVLEntryInit(entry);
    if (tree->root == NULL) {
        tree->root = entry;
        return true;
    }
    return AVLTreeInsertEntry(tree, &entry, tree->root, NULL);
}



/*
* ������ɾ���ڵ��ƽ�����
*/
bool AVLTreeDeleteEntryFixup(AVLTree* tree, bool isCurLeft, AVLEntry* cur, AVLEntry* curParent) {
    // ɾ���ڵ��ڵ�ƽ�����ӿ��ܷ����仯������ά���ڵ�ƽ������
    AVLEntry* newSubRoot = NULL;
    int curBF = AVLEntryGetBalanceFactor(cur);
    if (isCurLeft) curBF--;
    else curBF++;

    if (curBF != 0) {
        if (RotateByBalanceFactor(tree, NULL, cur, curBF, curParent) == false) {
            // ��һ��߶���Ȼ������������ת����ǰ�ڵ�߶Ȳ���
            return false;
        }
    }
    else {
        AVLEntrySetBalanceFactor(cur, curBF);
    }
    return true;
}


static bool DeleteMinEntry(AVLTree* tree, AVLEntry* cur, AVLEntry** curParent, AVLEntry** delete_minEntry, AVLEntry* deleteParent) {
    if (AVLEntryGetLeft(cur)) {
        if (DeleteMinEntry(tree, AVLEntryGetLeft(cur), &cur, delete_minEntry, deleteParent)) {
            return AVLTreeDeleteEntryFixup(tree, true, cur, *curParent);
        }
    } else {
        // ��С�ڵ�̳д�ɾ���ڵ������������Ϊ��С�ڵ�϶�û����ڵ㣬����ֱ�Ӹ�ֵ
        AVLEntrySetLeft(cur, AVLEntryGetLeft(*delete_minEntry));

        // ��С�ڵ�ҽӵ���Ҫɾ����λ��
        AVLTreeHitchEntry(tree, *delete_minEntry, deleteParent, cur);

        AVLEntrySetBalanceFactor(cur, AVLEntryGetBalanceFactor(*delete_minEntry));

        // ��С�ڵ�����Ǵ�ɾ���ڵ���ҽڵ�
        if (AVLEntryGetRight(*delete_minEntry) != cur) {
            // ��minEntry��ԭ�ȵ�λ��ժ������������������
            AVLEntrySetLeft(*curParent, cur->right);

            // ��С�ڵ�̳д�ɾ���ڵ��������
            cur->right = (*delete_minEntry)->right;
        } else {
            *curParent = cur;       // curParent����ָ��Ҫ���ص�minEntry��Ҳ�������ϲ�DeleteMinEntry��cur
        }
        *delete_minEntry = cur;     // ������С�ڵ�
        return true;
    }
}

/*
* �ݹ���Ҳ�ɾ���ڵ�
*/
bool AVLTreeDeleteEntry(AVLTree* tree, void* key, AVLEntry* cur, AVLEntry** curParent, AVLEntry** deleteEntry) {
    void* obj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
    int res = tree->cmpFunc(GetFieldByFieldOffset(obj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
    bool ret;
    bool isCurLeft;
    if (res > 0) {
        ret = AVLTreeDeleteEntry(tree, key, AVLEntryGetLeft(cur), &cur, deleteEntry);
        isCurLeft = true;
    }
    else if (res < 0) {
        ret = AVLTreeDeleteEntry(tree, key, cur->right, &cur, deleteEntry);
        isCurLeft = false;
    }
    else {
        *deleteEntry = cur;
        if (AVLEntryGetLeft(cur) != NULL && AVLEntryGetRight(cur) != NULL) {
            // �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
            // �ݹ�������
            AVLEntry* minEntry = cur;       // ����ɾ���ڵ㣬������С�ڵ�
            // ֮���Զ�����minEntry�ĵ�ַ����Ϊ�˱�֤�滻��С�ڵ�󣬵�һ�����ջ�ϵ�curParentҲ��ͬ��Ϊ��С�ڵ�
            ret = DeleteMinEntry(tree, AVLEntryGetRight(cur), &minEntry, &minEntry, *curParent);
            cur = minEntry;     // ��ǰ�ڵ��Ϊ��С�ڵ�
            isCurLeft = false;
        }
        else {
            if (cur->right != NULL) {
                // ֻ�����ӽڵ�
                AVLTreeHitchEntry(tree, cur, *curParent, cur->right);
                isCurLeft = false;
            }
            else if (AVLEntryGetLeft(cur) != NULL) {
                AVLTreeHitchEntry(tree, cur, *curParent, AVLEntryGetLeft(cur));
                isCurLeft = true;
            }
            else {
                // û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
                AVLTreeHitchEntry(tree, cur, *curParent, NULL);
                return true;
            }
            ret = true;
        }
    }
    if (ret) {
        ret = AVLTreeDeleteEntryFixup(tree, isCurLeft, cur, *curParent);
    }
    return ret;
}

/*
* �����а�keyɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
*/
AVLEntry* AVLTreeDeleteEntryByKey(AVLTree* tree, void* key) {
    AVLEntry* deleteEntry, * curParent = NULL;
    AVLTreeDeleteEntry(tree, key, tree->root, &curParent, &deleteEntry);
    return deleteEntry;
}