#include "CUtils\container\bs_tree_np.h"

/*
* newEntry����entry�ҽӵ�parent��
* newEntry�������ӽڵ㲻��
* entry������ժ��
* entry��left��right����
*/
static void BSTreeHitchEntry(BSTree* tree, BSEntry* entry, BSEntry* entryParent, BSEntry* newEntry) {
    if (entryParent) {
        if (entryParent->left == entry) {
            entryParent->left = newEntry;
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
* ��ʼ����
*/
void BSTreeInit(BSTree* tree, int entryFieldOffset, int keyFieldOffset, int keySize, CmpFunc cmpFunc) {
    tree->root = NULL;
    tree->entryFieldOffset = entryFieldOffset;
    tree->keyFieldOffset = keyFieldOffset;
    tree->keyFieldSize = keySize;
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
}


/*
* �ݹ������в���ڵ�
*/
bool BSTreeInsertEntry(BSTree* tree, BSEntry* entry, BSEntry* cur) {
    void* obj = GetObjByFieldOffset(entry, tree->entryFieldOffset, void);
    void* key = GetFieldByFieldOffset(obj, tree->keyFieldOffset, void);

    void* curObj = GetObjByFieldOffset(cur, tree->entryFieldOffset, void);
    int res = tree->cmpFunc(GetFieldByFieldOffset(curObj, tree->keyFieldOffset, void), key, tree->keyFieldSize);
    if (res == 0) {
        return false;
    }
    else if (res < 0) {
        if (cur->right) {
            return BSTreeInsertEntry(tree, entry, cur->right);
        }
        cur->right = entry;
    }
    else {
        if (cur->left) {
            return BSTreeInsertEntry(tree, entry, cur->left);
        }
        cur->left = entry;
    }
    return true;
}

/*
* �����в���ڵ�
* ����������ظ��ڵ�
* �ɹ�����true��ʧ�ܷ���false
*/
bool BSTreeInsertEntryByKey(BSTree* tree, BSEntry* entry) {
    BSEntryInit(entry);
    if (tree->root == NULL) {
        tree->root = entry;
        return true;
    }
    return BSTreeInsertEntry(tree, entry, tree->root);
}

/*
* ������ɾ���ڵ�
* �ɹ����ر�ɾ���Ľڵ㣬ʧ�ܷ���NULL
*/
BSEntry* BSTreeDeleteEntry(BSTree* tree, BSEntry* entry, BSEntry* entryParent) {
    if (entry->left != NULL && entry->right != NULL) {
        // �����Ҹ����ӽڵ㣬�ҵ�ǰ�ڵ������������С�Ľڵ㣬����С�ڵ��滻����ǰ�ڵ����ڵ�λ�ã�ժ����ǰ�ڵ㣬�൱���Ƴ�����С�ڵ�
        BSEntry* minEntry = entry->right;
        BSEntry* minParent = NULL;
        while (minEntry->left) {
            minParent = minEntry;
            minEntry = minEntry->left;
        }

        // ��С�ڵ�̳д�ɾ���ڵ������������Ϊ��С�ڵ�϶�û����ڵ㣬����ֱ�Ӹ�ֵ
        minEntry->left = entry->left;

        // ��С�ڵ�����Ǵ�ɾ���ڵ���ҽڵ�
        if (entry->right != minEntry) {
            // ��minEntry��ԭ�ȵ�λ��ժ������������������
            minParent->left = minEntry->right;

            // ��С�ڵ�̳д�ɾ���ڵ��������
            minEntry->right = entry->right;

            // �����Ҫ���ݣ������Ӧentry�ĸ�����minEntry�ĸ��׵������������ֱ���޸�entry��parent����Ϊ��û�ҽ�
        }
        else {
            // �����Ҫ���ݣ������Ӧentry�ĸ�����minEntry�������������ֱ���޸�entry��parent����Ϊ��û�ҽ�
        }
        // �����йҽ�
        BSTreeHitchEntry(tree, entry, entryParent, minEntry);

        // Ҳ����ѡ��ֱ�ӽ��������ڵ������
    }
    else {
        if (entry->right != NULL) {
            // ֻ�����ӽڵ�
            BSTreeHitchEntry(tree, entry, entryParent, entry->right);
        }
        else if (entry->left != NULL) {
            BSTreeHitchEntry(tree, entry, entryParent, entry->left);
        }
        else {
            // û���ӽڵ㣬ֱ�ӴӸ��ڵ���ժ���˽ڵ�
            BSTreeHitchEntry(tree, entry, entryParent, NULL);
        }
    }
    return entry;
}