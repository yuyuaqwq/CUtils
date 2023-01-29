/*
* @yuyuaqwq - 鱼鱼
* email:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* 请保留此声明
*/

#include "rb_tree.h"

typedef enum {
    kBlack,
    kRed,
} RBColor;


/*
* 左旋子树
*/
static RBEntry* RotateLeft(RBEntry* subRoot) {
    RBEntry* newSubRoot = subRoot->right;
    if (newSubRoot == NULL) {
        return subRoot;
    }

    RBEntrySetParent(newSubRoot, RBEntryGetParent(subRoot));
    if (RBEntryGetParent(subRoot)) {
        if (RBEntryGetParent(subRoot)->left == subRoot) {
            RBEntryGetParent(subRoot)->left = newSubRoot;
        }
        else {
            RBEntryGetParent(subRoot)->right = newSubRoot;
        }
    }
    RBEntrySetParent(subRoot, newSubRoot);

    subRoot->right = newSubRoot->left;
    if (subRoot->right) {
        RBEntrySetParent(subRoot->right, subRoot);
    }

    newSubRoot->left = subRoot;

    return newSubRoot;
}

/*
* 右旋子树
*/
static RBEntry* RotateRight(RBEntry* subRoot) {
    RBEntry* newSubRoot = subRoot->left;
    if (newSubRoot == NULL) {
        return subRoot;
    }

    RBEntrySetParent(newSubRoot, RBEntryGetParent(subRoot));
    if (RBEntryGetParent(subRoot)) {
        if (RBEntryGetParent(subRoot)->left == subRoot) {
            RBEntryGetParent(subRoot)->left = newSubRoot;
        }
        else {
            RBEntryGetParent(subRoot)->right = newSubRoot;
        }
    }
    RBEntrySetParent(subRoot, newSubRoot);

    subRoot->left = newSubRoot->right;
    if (subRoot->left) {
        RBEntrySetParent(subRoot->left, subRoot);
    }

    newSubRoot->right = subRoot;

    return newSubRoot;
}

/*
* 取兄弟节点
*/
static RBEntry* GetSiblingEntry(RBEntry* entry) {
    if (RBEntryGetParent(entry)->left == entry) {
        return RBEntryGetParent(entry)->right;
    }
    return RBEntryGetParent(entry)->left;
}

/*
* newEntry代替entry挂接到其父节点下
* newEntry的左右子节点不变
* entry从树中摘除
* entry的parent、left和right不变
*/
static void RBTreeHitchEntry(RBTree* tree, RBEntry* entry, RBEntry* newEntry) {
    if (RBEntryGetParent(entry)) {
        if (RBEntryGetParent(entry)->left == entry) {
            RBEntryGetParent(entry)->left = newEntry;
        }
        else {
            RBEntryGetParent(entry)->right = newEntry;
        }
    }
    if (newEntry) {
        RBEntrySetParent(newEntry, RBEntryGetParent(entry));
    }
    if (tree->root == entry) {
        tree->root = newEntry;
    }
}



inline RBColor RBEntryGetColor(RBEntry* entry) {
    return (RBColor)(((uintptr_t)entry->parent_color) & 0x1);
}

inline void RBEntrySetColor(RBEntry* entry, RBColor color) {
    entry->parent_color = (RBEntry*)(((uintptr_t)RBEntryGetParent(entry)) | ((uintptr_t)color));
}

inline RBEntry* RBEntryGetParent(RBEntry* entry) {
    return (RBEntry*)(((uintptr_t)entry->parent_color) & (~((uintptr_t)0x1)));
}

inline void RBEntrySetParent(RBEntry* entry, RBEntry* parent) {
    entry->parent_color = (RBEntry*)(((uintptr_t)parent) | ((uintptr_t)RBEntryGetColor(entry)));
}


/*
* 初始化树
*/
void RBTreeInit(RBTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
    BSTreeInit(&tree->bst, entryFieldOffset, keyFieldOffset, keySize, cmpFunc);
}

/*
* 初始化节点
*/
void RBEntryInit(RBEntry* entry, RBColor color) {
    BSEntryInit(&entry->bse);
    RBEntrySetColor(entry, color);
}

/*
* 从树中查找节点
* 存在返回查找到的节点，不存在返回NULL
*/
RBEntry* RBTreeFindEntryByKey(RBTree* tree, void* key) {
    return (RBEntry*)BSTreeFindEntryByKey(&tree->bst, key);
}

/*
* 向树中插入节点后的平衡操作
*/
void RBTreeInsertEntryBalance(RBTree* tree, RBEntry* entry) {
    RBEntry* cur = RBEntryGetParent(entry);
    if (cur == NULL) {
        RBEntrySetColor(entry, kBlack);
        return true;
    }
    RBEntrySetColor(entry, kRed);

    // 开始回溯维护
    while (cur) {
        if (RBEntryGetColor(cur) == kBlack) {
            // 当前节点(插入节点的父节点)是黑色，啥都不用做(是2节点/3节点的插入，直接合并)
            break;
        }

        if (RBEntryGetParent(cur) == NULL) {
            // 没有父节点，回溯到根节点了，直接染黑
            RBEntrySetColor(cur, kBlack);
            break;
        }
        RBEntry* sibling = GetSiblingEntry(cur);
        
        if (sibling && RBEntryGetColor(sibling) == kRed) {
            // 兄弟节点是红色，说明是4节点的插入，分裂(红黑树的体现就是变色)，父节点向上插入，继续回溯
            RBEntrySetColor(cur, kBlack);
            RBEntrySetColor(sibling, kBlack);
            entry = RBEntryGetParent(cur);       // 更新为该节点向上插入
            if (RBEntryGetParent(entry) == NULL) {
                RBEntrySetColor(entry, kBlack);     // 根节点，染黑并结束
                break;
            }
            RBEntrySetColor(entry, kRed);
            cur = entry;
        }
        else {
            // 没有兄弟节点或兄弟节点是黑色，说明是3节点的插入，可以并入，但需要利用旋转将其变为4节点
            //         10b               5b    
            //      5r     20b  ->   !2r     10r    
            //  !2r                             20b
            RBEntry* newSubRoot;
            RBEntry* oldSubRoot = RBEntryGetParent(cur);
            if (RBEntryGetParent(cur)->left == cur) {
                if (cur->right == entry) {
                    RotateLeft(cur);
                }
                newSubRoot = RotateRight(oldSubRoot);
            }
            else {
                if (cur->left == entry) {
                    RotateRight(cur);
                }
                newSubRoot = RotateLeft(oldSubRoot);
            }
            RBEntrySetColor(newSubRoot, kBlack);
            RBEntrySetColor(oldSubRoot, kRed);

            if (tree->root == oldSubRoot) {        // 原来的子树根节点可能是整棵树的根节点，因此要检查更新
                tree->root = newSubRoot;
            }

            break;        // 只是并入，未分裂，向上没有改变颜色，不再需要回溯
        }
        cur = RBEntryGetParent(cur);
    }
}

/*
* 向树中插入节点
* 不允许存在重复节点
* 成功返回true，失败返回false
*/
bool RBTreeInsertEntryByKey(RBTree* tree, RBEntry* entry) {
    if (!BSTreeInsertEntry(&tree->bst, &entry->bse)) {
        return false;
    }
    RBTreeInsertEntryBalance(tree, entry);
    return true;
}

/*
* 从树中删除节点
*/
void RBTreeDeleteEntry(RBTree* tree, RBEntry* entry) {
    // 从entry的父节点开始回溯
    RBEntry* parent;
    bool isLeft;
    if (entry->left != NULL && entry->right != NULL) {
        // 有左右各有子节点，找当前节点的右子树中最小的节点，用最小节点替换到当前节点所在的位置，摘除当前节点，相当于移除了最小节点
        RBEntry* minEntry = entry->right;
        while (minEntry->left) {
            minEntry = minEntry->left;
        }

        isLeft = RBEntryGetParent(minEntry)->left == minEntry;

        // 最小节点继承待删除节点的左子树，因为最小节点肯定没有左节点，所以直接赋值
        minEntry->left = entry->left;
        if (entry->left) {
            RBEntrySetParent(entry->left, minEntry);
        }

        RBEntry* oldRight = minEntry->right;

        // 最小节点可能是待删除节点的右节点
        if (entry->right != minEntry) {
            // 将minEntry从原先的位置摘除，用其右子树代替
            RBEntryGetParent(minEntry)->left = minEntry->right;
            if (minEntry->right) {
                RBEntrySetParent(minEntry->right, AVLEntryGetParent(minEntry));
            }
            // 最小节点继承待删除节点的右子树
            minEntry->right = entry->right;
            if (entry->right) {
                RBEntrySetParent(entry->right, minEntry);
            }
            parent = RBEntryGetParent(minEntry);
        }
        else {
            parent = minEntry;
        }

        RBColor oldColor = RBEntryGetColor(minEntry);
        RBEntrySetColor(minEntry, RBEntryGetColor(entry));

        // 红黑树另外需要使entry的子节点同步为minEntry原来的子节点，在判断是否存在子节点时使用
        entry->left = NULL;
        entry->right = oldRight;
        RBEntrySetColor(entry, oldColor);

        // 最后进行挂接
        RBTreeHitchEntry(tree, entry, minEntry);
    }
    else {
        parent = RBEntryGetParent(entry);
        if (parent) {
            isLeft = parent->left == entry;
        }

        if (entry->right != NULL) {
            // 只有右子节点
            RBTreeHitchEntry(tree, entry, entry->right);
        }
        else if (entry->left != NULL) {
            RBTreeHitchEntry(tree, entry, entry->left);
        }
        else {
            // 没有子节点，直接从父节点中摘除此节点
            RBTreeHitchEntry(tree, entry, NULL);
        }
    }

    if (entry) {
        if (RBEntryGetColor(entry) == kRed) {
            // 是红色的，是3/4节点，因为此时一定是叶子节点(红节点不可能只有一个子节点)，直接移除
            parent = NULL;
        }
        // 是黑色的，但是有一个子节点，说明是3节点，变为2节点即可
        else if (entry->left) {
            RBEntrySetColor(entry->left, kBlack);
            parent = NULL;
        }
        else if (entry->right) {
            RBEntrySetColor(entry->right, kBlack);
            parent = NULL;
        }
    }

    RBEntry* newSubRoot;
    // 回溯维护删除黑色节点，即没有子节点(2节点)的情况
    while (parent) {
        RBEntry* sibling = isLeft ? parent->right : parent->left;
        if (RBEntryGetColor(sibling) == kRed) {
            // 兄弟节点为红，说明兄弟节点与父节点形成3节点，真正的兄弟节点应该是红兄弟节点的子节点
            // 旋转，此时只是使得兄弟节点和父节点形成的3节点红色链接位置调换，当前节点的兄弟节点变为原兄弟节点的子节点
            RBEntry* oldSubRoot = RBEntryGetParent(sibling);
            RBEntrySetColor(oldSubRoot, kRed);
            RBEntrySetColor(sibling, kBlack);
            if (oldSubRoot->left == sibling){
                newSubRoot = RotateRight(oldSubRoot);
                sibling = oldSubRoot->left;     // 下降后挂接过来的节点
            } else {
                newSubRoot = RotateLeft(oldSubRoot);
                sibling = oldSubRoot->right;     // 下降后挂接过来的节点
            }
            if (tree->root == oldSubRoot) {
                tree->root = newSubRoot;
            }
        }

        // 至此兄弟节点一定为黑
        
        // 侄子节点为红，即兄弟节点是3/4节点的情况，向兄弟借节点(上升兄弟节点，下降父亲节点)
        if (sibling->right && RBEntryGetColor(sibling->right) == kRed || sibling->left && RBEntryGetColor(sibling->left) == kRed) {
            RBColor parentColor = RBEntryGetColor(parent);
            RBEntrySetColor(parent, kBlack);
            RBEntry* oldSubRoot = parent;
            if (parent->left == sibling) {
                if (!sibling->left || RBEntryGetColor(sibling->left) == kBlack) {
                    RBEntrySetColor(sibling->right, kBlack);
                    sibling = RotateLeft(sibling);
                } else {
                    RBEntrySetColor(sibling->left, kBlack);
                }
                newSubRoot = RotateRight(parent);
            }
            else {
                if (!sibling->right || RBEntryGetColor(sibling->right) == kBlack) {
                    RBEntrySetColor(sibling->left, kBlack);
                    sibling = RotateRight(sibling);
                } else {
                    RBEntrySetColor(sibling->right, kBlack);
                }
                newSubRoot = RotateLeft(parent);
            }
            // 该节点会接替原先的子根节点，也要接替颜色
            RBEntrySetColor(sibling, parentColor);
            if (tree->root == oldSubRoot) {
                tree->root = newSubRoot;
            }
            break;
        }
        
        if (RBEntryGetColor(parent) == kRed) {
            // 父节点为红，即父节点是3/4节点，分裂下降与兄弟节点合并
            //    |5|8|               |5|
            //   /  |  \     ->      /   \
            //  3   6  -9-          3   |6|8|
            RBEntrySetColor(sibling, kRed);
            RBEntrySetColor(parent, kBlack);
            break;
        }
        else {
            // 父节点为黑，即父节点是2节点，兄弟节点也是2节点，下降父节点与兄弟节点合并，相当于向上删除父节点，继续回溯
            // 为什么不是3/4节点？因为黑父节点如果是3，兄弟节点是红，4的话回溯时父节点是红
            RBEntrySetColor(sibling, kRed);
        }
        RBEntry* child = parent;
        parent = RBEntryGetParent(parent);
        if (parent) {
            isLeft = parent->left == child;
        }
    }

    if (tree->root && RBEntryGetColor(tree->root) == kRed) {
        // 根节点染黑
        RBEntrySetColor(tree->root, kBlack);
    }
}

/*
* 从树中按key删除节点
* 成功返回被删除的节点，失败返回NULL
*/
RBEntry* RBTreeDeleteEntryByKey(RBTree* tree, void* key) {
    RBEntry* entry = RBTreeFindEntryByKey(tree, key);
    if (entry) {
        RBTreeDeleteEntry(tree, entry);
    }
    return entry;
}