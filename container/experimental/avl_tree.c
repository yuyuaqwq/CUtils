#include <CUtils/container/experimental/avl_tree.h>

#ifndef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
/*
* 获取父节点
*/
inline AvlEntry* AvlEntryGetParent(AvlEntry* entry) {
    return (AvlEntry*)(((uintptr_t)entry->parent_balanceFactor) & (~((uintptr_t)0x3)));
}

/*
* 设置父节点
*/
inline void AvlEntrySetParent(AvlEntry* entry, AvlEntry* parent) {
    entry->parent_balanceFactor = (AvlEntry*)(((uintptr_t)parent) | ((uintptr_t)entry->parent_balanceFactor & 0x3));
}

/*
* 获取节点平衡因子
*/
inline int AvlEntryGetBalanceFactor(AvlEntry* entry) {
    int ret = (int)(((uintptr_t)entry->parent_balanceFactor) & 0x3);
    if (ret == 3) {
        ret = -1;
    }
    return ret;
}

/*
* 设置节点平衡因子
*/
inline void AvlEntrySetBalanceFactor(AvlEntry* entry, int balanceFactor) {
    entry->parent_balanceFactor = (AvlEntry*)(((uintptr_t)AvlEntryGetParent(entry)) | ((uintptr_t)balanceFactor) & 0x3);
}

#else
/*
* 获取父节点
*/
inline AvlEntry* AvlEntryGetParent(AvlEntry* entry) {
    return entry->parent;
}

/*
* 设置父节点
*/
inline void AvlEntrySetParent(AvlEntry* entry, AvlEntry* parent) {
    entry->parent = parent;
}

/*
* 根据左右子树高度获取节点平衡因子
*/
int AvlEntryGetBalanceFactor(AvlEntry* subRoot) {
    int leftHeight = -1, rightHeight = -1;
    if (subRoot->left) leftHeight = subRoot->left->height;
    if (subRoot->right) rightHeight = subRoot->right->height;
    return leftHeight - rightHeight;
}


/*
* 根据左右子树高度获取节点高度
*/
static int GetMaxHeight(AvlEntry* subRoot) {
    int leftHeight = -1, rightHeight = -1;
    if (subRoot->left) leftHeight = subRoot->left->height;
    if (subRoot->right) rightHeight = subRoot->right->height;
    return max(leftHeight, rightHeight) + 1;
}

/*
* 根据左右子树高度更新节点高度
* 更新完成返回true，无需更新返回false
*/
static bool UpdateHeight(AvlEntry* subRoot) {
    int newHeight = GetMaxHeight(subRoot);
    if (subRoot->height != newHeight) {
        subRoot->height = newHeight;
        return true;
    }
    return false;
}
#endif // CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_

/*
* 获取左子节点
*/
inline AvlEntry* AvlEntryGetLeft(AvlEntry* entry) {
    return entry->left;
}

/*
* 设置左子节点
*/
inline void AvlEntrySetLeft(AvlEntry* entry, AvlEntry* left) {
    entry->left = left;
}

/*
* 获取右子节点
*/
inline AvlEntry* AvlEntryGetRight(AvlEntry* entry) {
    return entry->right;
}

/*
* 设置右子节点
*/
inline void AvlEntrySetRight(AvlEntry* entry, AvlEntry* right) {
    entry->right = right;
}


/*
* 左旋子树
*/
static AvlEntry* RotateLeft(AvlEntry* subRoot) {
    AvlEntry* newSubRoot = AvlEntryGetRight(subRoot);
    if (newSubRoot == NULL) {
        return subRoot;
    }

    AvlEntrySetParent(newSubRoot, AvlEntryGetParent(subRoot));
    if (AvlEntryGetParent(subRoot)) {
        if (AvlEntryGetLeft(AvlEntryGetParent(subRoot)) == subRoot) {
            AvlEntrySetLeft(AvlEntryGetParent(subRoot), newSubRoot);
        }
        else {
            AvlEntrySetRight(AvlEntryGetParent(subRoot), newSubRoot);
        }
    }
    AvlEntrySetParent(subRoot, newSubRoot);

    AvlEntrySetRight(subRoot, AvlEntryGetLeft(newSubRoot));
    if (AvlEntryGetRight(subRoot)) {
        AvlEntrySetParent(AvlEntryGetRight(subRoot), subRoot);
    }

    AvlEntrySetLeft(newSubRoot, subRoot);

#ifdef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
    UpdateHeight(subRoot);
    UpdateHeight(newSubRoot);
#endif // CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_

    return newSubRoot;
}

/*
* 右旋子树
*/
static AvlEntry* RotateRight(AvlEntry* subRoot) {
    AvlEntry* newSubRoot = AvlEntryGetLeft(subRoot);
    if (newSubRoot == NULL) {
        return subRoot;
    }

    AvlEntrySetParent(newSubRoot, AvlEntryGetParent(subRoot));
    if (AvlEntryGetParent(subRoot)) {
        if (AvlEntryGetLeft(AvlEntryGetParent(subRoot)) == subRoot) {
            AvlEntrySetLeft(AvlEntryGetParent(subRoot), newSubRoot);
        }
        else {
            AvlEntrySetRight(AvlEntryGetParent(subRoot), newSubRoot);
        }
    }
    AvlEntrySetParent(subRoot, newSubRoot);

    AvlEntrySetLeft(subRoot, AvlEntryGetRight(newSubRoot));
    if (AvlEntryGetLeft(subRoot)) {
        AvlEntrySetParent(AvlEntryGetLeft(subRoot), subRoot);
    }

    AvlEntrySetRight(newSubRoot, subRoot);

#ifdef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
    UpdateHeight(subRoot);
    UpdateHeight(newSubRoot);
#endif // CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_

    return newSubRoot;

}

/*
* newEntry代替entry挂接到其父节点下
* newEntry的左右子节点不变
* entry从树中摘除
* entry的parent、left和right不变
*/
static void AvlTreeHitchEntry(AvlTree* tree, AvlEntry* entry, AvlEntry* newEntry) {
    if (AvlEntryGetParent(entry)) {
        if (AvlEntryGetLeft(AvlEntryGetParent(entry)) == entry) {
            AvlEntrySetLeft(AvlEntryGetParent(entry), newEntry);
        }
        else {
            AvlEntrySetRight(AvlEntryGetParent(entry), newEntry);
        }
    }
    if (newEntry) {
        AvlEntrySetParent(newEntry, AvlEntryGetParent(entry));
    }
    if (tree->root == entry) {
        tree->root = newEntry;
    }
}



#ifndef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
/*
* 旋转平衡因子调整(右旋为例)：
* 单旋情况：
*
* 原根节点的平衡因子是2，新根节点的平衡因子是1
* 说明原根节点的右子树深度是小于左子树2的
*
* 情况1：平衡因子置0
*                 o6, 2                    o5, 0
*         o5, 1         o3    ->      o4          o6(4), 0
*     o4     o3                                o3        o3
* 新根节点的右子树(o3)高度一定低于新根节点2，与原根节点(o6)的右子树高度相等，因此原根节点的平衡因子置0
*
*
*
* 情况2：调整平衡因子(左旋为例)
* 删除节点时可能会出现新根节点平衡因子为0的情况，如下删除的是o1
*                    o4, -2                                    o3(4), 1
*         o2(1), 0           o3, 0       ->           o4(3), -1            o2
*              -o1-       o2       o2         o2(1), 0         o2
* 此时不能简单置0
* 原根节点(o4)高度高于新根节点(o3)子树(o2, o2)1，而o4的左子树(o2(1))又低于o3子树(o2, o2)1
* 故原根节点(o4)置为-1
* o4高度高于o3子树1，而o4的左子树又低于o3子树1
*
* 因为挂接过去的节点高度可能与当前高度不同，因此需要调整
*
*
*
*
* 双旋情况：
*
* 原根节点的平衡因子是2，新根节点的平衡因子是-1
* 直接旋转：
*                 o6, 2                       o5(6), -2
*         o5, -1         o3    ->       o3                 o6(5), 1
*     o3     o4                                       o4          o3
*
*
* 先左旋：
* 情况1：
*                      o6, 2                                         o6, 2
*            o5, -1            o3    ->                  o4(5), 1               o3
*     o3            o4, -1                        o5(4),1          o3
*              o2         o3                 o3          o2
* o4的子节点高度是o2或者o3，挂到o5下可能影响o5的平衡因子，因此右旋后需要调整o5的平衡因子
*
* 依据是o4，-1时(o2和o3)置为1，0(o3和o3)和1(o3和o2)时置为0
* o5的新右子节点是o4的左子节点，o4的左子节点要么o2要么o3
*
*
* 情况2：
*
*                      o6, 2                                         o6, 2
*            o5, -1            o3    ->                  o4(5), 2               o3
*     o3            o4, 1                        o5(4),0          o2
*              o3         o2                o3          o3
* 情况2是第一次旋完之后，o4的右子节点o2与o6的右子节点o3不同高度
* 即相当于旋完后还需要更新o6的平衡因子
* 依据是o4，1时(o3和o2，o2要给o6做左子节点)置为-1
*
*/

/*
* 根据平衡因子来旋转子树
* 并负责更新平衡因子
* 子树高度变化返回true，高度未变化返回false
*/
static bool RotateByBalanceFactor(AvlTree* tree, AvlEntry** subRoot_, int curBF) {
    AvlEntry* subRoot = *subRoot_;

    bool rotate = false;
    bool heightUpdate = true;
    AvlEntry* newSubRoot = NULL;
    if (curBF == 2) {
        // 是失衡节点(最小失衡子树的根节点)，左子树高度高于右子树高度
        AvlEntry* child = subRoot->left;
        int childBF = AvlEntryGetBalanceFactor(child);
        // 判断需要单旋还是双旋
        if (childBF == -1) {
            // 失衡节点的左子树的右子树更深，先对失衡节点的左子树左旋，再对失衡节点右旋
            //      o
            //  o
            //    o
            // 需要先左旋转
            int rightBF = AvlEntryGetBalanceFactor(child->right);
            if (child->right) {
                AvlEntrySetBalanceFactor(child->right, 0);
            }

            RotateLeft(child);
            newSubRoot = RotateRight(subRoot);

            if (rightBF == -1) {
                AvlEntrySetBalanceFactor(child, 1);
                AvlEntrySetBalanceFactor(subRoot, 0);
            }
            else {
                AvlEntrySetBalanceFactor(child, 0);
                if (rightBF == 1) {
                    AvlEntrySetBalanceFactor(subRoot, -1);
                } else {
                    AvlEntrySetBalanceFactor(subRoot, 0);
                }
            }
        }
        else if (childBF == 0) {
            // 删除节点时可能出现的情况，即右子树的右子树高度低于左子树的左子树1，旋转过来并不完美平衡
            //      o
            //   o
            // o   o

            //       o4, 2
            // o3, 0       o2(1)
            heightUpdate = false;       // 这种情况的旋转不会改变该子树的高度
            newSubRoot = RotateRight(subRoot);
            AvlEntrySetBalanceFactor(child, -1);
            AvlEntrySetBalanceFactor(subRoot, 1);
        }
        else {
            // 此时失衡节点的左子树的左子树更深，右旋即可
            // 可能失衡节点与左节点交换位置，需要保存结果，如果是失衡节点是根节点再返回新的根节点
            newSubRoot = RotateRight(subRoot);
            AvlEntrySetBalanceFactor(child , 0);
            AvlEntrySetBalanceFactor(subRoot, 0);
        }
        rotate = true;
    }
    else if (curBF == -2) {
        // 是失衡节点，右子树高度高于左子树高度
        AvlEntry* child = subRoot->right;
        int childBF = AvlEntryGetBalanceFactor(child);
        // 判断需要单旋还是双旋
        if (childBF == 1) {
            // 失衡节点的右子树的左子树更深，先对失衡节点的右子树右旋，再对失衡节点左旋
            //    o
            //        o
            //      o
            // 需要先右旋转
            int leftBF = AvlEntryGetBalanceFactor(child->left);
            if (child->left) {
                AvlEntrySetBalanceFactor(child->left, 0);
            }

            RotateRight(child);
            newSubRoot = RotateLeft(subRoot);

            if (leftBF == 1) {
                AvlEntrySetBalanceFactor(child, -1);
                AvlEntrySetBalanceFactor(subRoot, 0);
            }
            else {
                AvlEntrySetBalanceFactor(child, 0);
                if (leftBF == -1) {
                    AvlEntrySetBalanceFactor(subRoot, 1);
                } else {
                    AvlEntrySetBalanceFactor(subRoot, 0);
                }
            }
        }
        else if (childBF == 0) {
            // 删除节点时可能出现的情况，即左子树的左子树高度低于右子树的右子树1，旋转过来并不完美平衡
            //    o
            //        o
            //      o   o

            //       o4, -2
            // o2(1)       o3, 0
            heightUpdate = false;       // 这种情况的旋转不会改变该子树的高度
            newSubRoot = RotateLeft(subRoot);
            AvlEntrySetBalanceFactor(child, 1);
            AvlEntrySetBalanceFactor(subRoot, -1);
        }
        else {      // childBF == -1
            // 此时失衡节点的右子树的右子树更深，左旋即可
            newSubRoot = RotateLeft(subRoot);
            AvlEntrySetBalanceFactor(child, 0);
            AvlEntrySetBalanceFactor(subRoot, 0);
        }
        rotate = true;
    }
    else {
        heightUpdate = false;
        AvlEntrySetBalanceFactor(subRoot, curBF);
    }

    if (rotate) {
        if (tree->root == subRoot) {        // 原来的子树根节点可能是整棵树的根节点，因此要检查更新
            tree->root = newSubRoot;
        }
        *subRoot_ = newSubRoot;
    }

    return heightUpdate;
}
#else
/*
* 根据平衡因子来旋转子树
* 子树高度变化返回true，子树高度未变化返回false
*/
static bool RotateByBalanceFactor(AvlTree* tree, AvlEntry** subRoot_) {
    AvlEntry* subRoot = *subRoot_;

    int factor = AvlEntryGetBalanceFactor(subRoot);
    bool rotate = false;
    bool hightUpdate = true;
    AvlEntry* newSubRoot = NULL;
    if (factor == 2) {
        // 是失衡节点(最小失衡子树的根节点)，左子树高度高于右子树高度

        // 判断需要单旋还是双旋
        if (AvlEntryGetBalanceFactor(subRoot->left) == -1) {
            // 失衡节点的左子树的右子树更深，先对失衡节点的左子树左旋，再对失衡节点右旋
            //      o
            //  o
            //    o
            // 需要先左旋转
            RotateLeft(subRoot->left);
        }
        else if (AvlEntryGetBalanceFactor(subRoot->left) == 0) {
            //      o
            //   o
            // o   o
            hightUpdate = false;        // 删除时可能出现的情况，旋转后高度不变
        }
        // 此时失衡节点的左子树的左子树更深，右旋即可
        // 可能失衡节点与左节点交换位置，需要保存结果，如果是失衡节点是根节点再返回新的根节点
        newSubRoot = RotateRight(subRoot);
        rotate = true;
    }
    else if (factor == -2) {
        // 是失衡节点，右子树高度高于左子树高度

        // 判断需要单旋还是双旋
        if (AvlEntryGetBalanceFactor(subRoot->right) == 1) {
            // 失衡节点的右子树的左子树更深，先对失衡节点的右子树右旋，再对失衡节点左旋
            //    o
            //        o
            //      o
            // 需要先右旋转
            RotateRight(subRoot->right);
        }
        else if (AvlEntryGetBalanceFactor(subRoot->right) == 0) {
            //    o
            //        o
            //      o   o
            hightUpdate = false;        // 删除时可能出现的情况，旋转后高度不变
        }
        // 此时失衡节点的右子树的右子树更深，左旋即可
        newSubRoot = RotateLeft(subRoot);
        rotate = true;
    }
    else {
        hightUpdate = false;
    }

    if (rotate) {
        if (tree->root == subRoot) {        // 原来的子树根节点可能是整棵树的根节点，因此要检查更新
            tree->root = newSubRoot;
        }
        *subRoot_ = newSubRoot;
    }

    return hightUpdate;
}

#endif // CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_


/*
* 向树中插入节点后的平衡操作
*/
void AvlTreeInsertEntryFixup(AvlTree* tree, AvlEntry* insertEntry) {
#ifndef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
    AvlEntry* cur = AvlEntryGetParent(insertEntry);
    AvlEntry* child = insertEntry;
    // 插入节点后平衡因子可能发生变化，回溯维护平衡因子
    while (cur) {
        int curBF = AvlEntryGetBalanceFactor(cur);
        if (child == cur->left) curBF++;        // 新节点插入到当前节点的左子树
        else curBF--;       // 新节点插入到当前节点的右子树

        if (RotateByBalanceFactor(tree, &cur, curBF) || curBF == 0) {
            // 旋转后当前节点高度不变，或原先高度就不变，停止回溯
            break;
        }
        child = cur;
        cur = AvlEntryGetParent(cur);
    }
#else
    AvlEntry* cur = AvlEntryGetParent(entry);

    // 插入节点后高度可能发生变化，回溯维护节点高度
    int heightCount = 1;
    while (cur) {
        if (cur->height >= heightCount) {
            break;        // 至此高度未发生变化，不再回溯
        }
        cur->height = heightCount++;
        if (RotateByBalanceFactor(tree, &cur)) {
            break;        // 插入后如果旋转了，就不需要再向上回溯了，因为旋转会导致这颗子树的高度不变
        }
        cur = AvlEntryGetParent(cur);
    }
#endif
}

/*
* 从树中删除节点的平衡操作
*/
void AvlTreeDeleteEntryFixup(AvlTree* tree, AvlEntry* cur, bool isCurLeft) {
#ifndef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
    // 删除节点后节点平衡因子可能发生变化，回溯维护节点平衡因子
    AvlEntry* newSubRoot = NULL;
    while (cur) {
        int curBF = AvlEntryGetBalanceFactor(cur);
        if (isCurLeft) curBF--;
        else curBF++;

        if (curBF != 0) {
            if (RotateByBalanceFactor(tree, &cur, curBF) == false) {
                // 另一侧高度相等或更深且无需旋转，则当前节点高度不变
                break;
            }
        }
        else {
            AvlEntrySetBalanceFactor(cur, curBF);
        }

        AvlEntry* temp = cur;
        cur = AvlEntryGetParent(cur);
        if (cur) {
            isCurLeft = cur->left == temp;
        }
    }

#else
    // 删除节点后高度可能发生变化，回溯维护节点高度
    while (parent) {
        if (UpdateHeight(parent) == false) {
            // 父节点高度未变化则说明，父节点的另一子树(兄弟子树)深度相等或更深，要检查是否失衡
            if (RotateByBalanceFactor(tree, &parent) == false) {
                break;        // 未失衡则停止回溯，从当前节点开始不会再影响上层节点的高度，旋转后要继续回溯，因为旋转可能会使得当前子树高度降低。
            }
        }
        parent = AvlEntryGetParent(parent);
    }
#endif
}




/*
* 初始化Avl树
*/
void AvlTreeInit(AvlTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
    BsTreeInit(&tree->bst, entryFieldOffset, keyFieldOffset, keySize, cmpFunc);
}

/*
* 初始化节点
*/
void AvlEntryInit(AvlEntry* entry) {
    BsEntryInit(&entry->bse);
#if !defined(CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_)
    AvlEntrySetBalanceFactor(entry, 0);
#else
    entry->height = 0;
#endif
}

/*
* 从树中查找节点
* 存在返回查找到的节点，不存在返回NULL
*/
void* AvlTreeFindEntryByKey(AvlTree* tree, void* key) {
    return BsTreeFindEntryByKey(&tree->bst, key);
}

/*
* 从树中按key插入节点
* 不允许存在重复节点
* 成功返回true，失败返回false
*/
bool AvlTreeInsertEntryByKey(AvlTree* tree, AvlEntry* insertEntry) {
    if (!BsTreeInsertEntryByKey(&tree->bst, &insertEntry->bse)) {
        return false;
    }
    AvlTreeInsertEntryFixup(tree, insertEntry);
    return true;
}

/*
* 从树中删除节点
*/
void AvlTreeDeleteEntry(AvlTree* tree, AvlEntry* deleteEntry) {
    // 从entry的父节点开始回溯
    AvlEntry* cur;
    bool isCurLeft;
    if (deleteEntry->left != NULL && deleteEntry->right != NULL) {
        // 有左右各有子节点，找当前节点的右子树中最小的节点，用最小节点替换到当前节点所在的位置，摘除当前节点，相当于移除了最小节点
        AvlEntry* minEntry = deleteEntry->right;
        while (minEntry->left) {
            minEntry = minEntry->left;
        }

        isCurLeft = AvlEntryGetParent(minEntry)->left == minEntry;

        // 最小节点继承待删除节点的左子树，因为最小节点肯定没有左节点，所以直接赋值
        minEntry->left = deleteEntry->left;
        if (deleteEntry->left) {
            AvlEntrySetParent(deleteEntry->left, minEntry);
        }

        // 最小节点可能是待删除节点的右节点
        if (deleteEntry->right != minEntry) {
            // 将minEntry从原先的位置摘除，用其右子树代替
            AvlEntryGetParent(minEntry)->left = minEntry->right;
            if (minEntry->right) {
                AvlEntrySetParent(minEntry->right, AvlEntryGetParent(minEntry));
            }
            // 最小节点继承待删除节点的右子树
            minEntry->right = deleteEntry->right;
            if (deleteEntry->right) {
                AvlEntrySetParent(deleteEntry->right, minEntry);
            }
            cur = AvlEntryGetParent(minEntry);
        }
        else {
            cur = minEntry;
        }

#ifndef CUTILS_CONTAINER_Avl_TREE_STORAGE_HEIGHT_H_
        AvlEntrySetBalanceFactor(minEntry, AvlEntryGetBalanceFactor(deleteEntry));
#else
        minEntry->height = entry->height;
#endif

        // 最后进行挂接
        AvlTreeHitchEntry(tree, deleteEntry, minEntry);
    }
    else {
        cur = AvlEntryGetParent(deleteEntry);
        if (cur) {
            isCurLeft = cur->left == deleteEntry;
        } else {
            isCurLeft = false;
        }

        if (deleteEntry->right != NULL) {
            // 只有右子节点
            AvlTreeHitchEntry(tree, deleteEntry, deleteEntry->right);
        }
        else if (deleteEntry->left != NULL) {
            AvlTreeHitchEntry(tree, deleteEntry, deleteEntry->left);
        }
        else {
            // 没有子节点，直接从父节点中摘除此节点
            AvlTreeHitchEntry(tree, deleteEntry, NULL);
        }
    }

    AvlTreeDeleteEntryFixup(tree, cur, isCurLeft);
}

/*
* 从树中按key删除节点
* 成功返回被删除的节点，失败返回NULL
*/
void* AvlTreeDeleteEntryByKey(AvlTree* tree, void* key) {
    void* obj = AvlTreeFindEntryByKey(tree, key);
    if (obj) {
        AvlEntry* entry = ObjectGetFieldByOffset(obj, tree->entryFieldOffset, void);
        AvlTreeDeleteEntry(tree, entry);
    }
    return obj;
}