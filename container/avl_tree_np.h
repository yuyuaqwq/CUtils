

/*
* @yuyuaqwq - ����
* email:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* �뱣��������
*/

#ifndef CUTILS_CONTAINER_AVLTREE_NP_H_
#define CUTILS_CONTAINER_AVLTREE_NP_H_

#include <CUtils/object.h>
#include <CUtils/container/bs_tree_np.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* �޸����Ƕ��ƽ�����ӵ�AVL��
*/
typedef struct _AVLEntry {
    union {
        struct {
            struct _AVLEntry* left_balanceFactor;     // ƽ������Ƕ�뵽��ָ���2λ
            struct _AVLEntry* right;
        };
        BSEntry bse;
    };
} AVLEntry;

typedef struct _AVLTree {
    union {
        struct {
            AVLEntry* root;
            int entryFieldOffset;
            int keyFieldOffset;
            int keyFieldSize;
            CmpFunc cmpFunc;        // ��ӵ�������һ������
        };
        BSTree bst;
    };
} AVLTree;

AVLEntry* AVLEntryGetLeft(AVLEntry* entry);
AVLEntry* AVLEntryGetRight(AVLEntry* entry);
int AVLEntryGetBalanceFactor(AVLEntry* entry);

void AVLTreeInit(AVLTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc);
#define AVLTreeInitByField(tree, objName, entryFieldName, keyFieldName) AVLTreeInit((tree), GetFieldOffset(objName, entryFieldName), GetFieldOffset(objName, keyFieldName), GetFieldSize(objName, keyFieldName), NULL)
void AVLEntryInit(AVLEntry* entry);
AVLEntry* AVLTreeFindEntryByKey(AVLTree* tree, void* key);
bool AVLTreeInsertEntryByKey(AVLTree* tree, AVLEntry* entry);
AVLEntry* AVLTreeDeleteEntryByKey(AVLTree* tree, void* key);


#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_AVLTREE_H_


