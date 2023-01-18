/*
* @yuyuaqwq - 鱼鱼
* emali:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* 请保留此声明
*/

#include "BPlusTree.h"
#include "CUtils/container/array.h"

typedef enum _BPlusEntryType {
    kIndex,
    kLeaf,
} BPlusEntryType;


#ifndef CUTILS_CONTAINER_BPLUS_TREE_MODE_DISK
PageId PageAlloc(int m) {
    return (PageId)MemoryAlloc(sizeof(BPlusEntry) - sizeof(BPlusLeafListEntry) + (m - 1) * sizeof(BPlusIndexInternalEntry) + sizeof(struct _BPlusIndexEntry*));
}

void PageFree(PageId id) {
    return MemoryFree((void*)id);
}

struct _BPlusEntry* PageGet(PageId id) {
    return (BPlusEntry*)id;
}
#endif // CUTILS_CONTAINER_BPLUS_TREE_DISK



static void LeafListEntryInit(PageId entryId) {
    BPlusEntry* first = PageGet(entryId);
    first->leafListEntry.next = entryId;
    first->leafListEntry.prev = entryId;
}

static void LeafListEntryInsertTail(PageId leftId, PageId rightId) {
    BPlusEntry* left = PageGet(leftId);
    BPlusEntry* right = PageGet(rightId);

    PageId temp = left->leafListEntry.next;
    left->leafListEntry.next = rightId;
    right->leafListEntry.next = temp;
    right->leafListEntry.prev = leftId;
    PageGet(temp)->leafListEntry.prev = rightId;
}

static void LeafListEntryRemoveTail(PageId entryId) {
    BPlusEntry* entry = PageGet(entryId);
    BPlusEntry* next = PageGet(entry->leafListEntry.next);
    entry->leafListEntry.next = next->leafListEntry.next;
    PageGet(next->leafListEntry.next)->leafListEntry.prev = entryId;
}


static inline BPlusLeafInternalEntry* GetLeafInternalEntry(BPlusEntry* leaf, int i) {
    return &leaf->leafInternalEntry[i];
}

static inline BPlusIndexInternalEntry* GetIndexInternalEntry(BPlusEntry* index, int i) {
    return &index->indexInternalEntry[i];
}


static PageId BPlusCreateIndexEntry(int m) {
    // child 应当是m个，所以最后多申请一个
    PageId entryId = PageAlloc(m);
    BPlusEntry* entry = PageGet(entryId);
    entry->type = kIndex;
    entry->parentId = NULL;
    for (int i = 0; i < m; i++) {
        GetIndexInternalEntry(entry, i)->childId = NULL;
    }
    entry->count = 0;
    return entryId;
}

static PageId BPlusCreateLeafEntry(int m) {
    PageId entryId = PageAlloc(m);
    BPlusEntry* entry = PageGet(entryId);
    entry->type = kLeaf;
    entry->parentId = NULL;
    for (int i = 0; i < m - 1; i++) {
        
    }
    entry->count = 0;
    return entryId;
}

/*
* 没有明确标准的索引是key索引
* 
* 通常情况下，父节点内部节点是左父索引，子节点是右子节点
* 即默认是key[0] - child[1]、key[1] - child[2]...
*         3     |    6    |    8
*      /        \         \        \
*   1|2         4|5        7        10|11
* 如上面画的示例，默认是3对应4|5，6对应7、8对应10|11
*/

/*
* 获取左兄弟节点，没有则返回NULL
*/
static PageId BPlusGetLeftSiblingEntry(BPlusTree* tree, PageId entryId, int leftParentIndex) {
    BPlusEntry* parent = PageGet(PageGet(entryId)->parentId);
    if (!parent) {
        return NULL;
    }
    if (leftParentIndex > -1) {
        return GetIndexInternalEntry(parent, leftParentIndex)->childId;
    }
    return NULL;
}

/*
* 获取右兄弟节点，没有则返回NULL
*/
static PageId BPlusGetRightSiblingEntry(BPlusTree* tree, PageId entryId, int leftParentIndex) {
    BPlusEntry* parent = PageGet(PageGet(entryId)->parentId);
    if (!parent) {
        return NULL;
    }
    if (leftParentIndex < parent->count) {
        return GetIndexInternalEntry(parent, leftParentIndex + 2)->childId;
    }
    return NULL;
}

/*
* 向内部节点数组指定位置插入key及子节点(是索引节点时)
*/
static void BPlusInsertInternalEntry(PageId entryId, int keyIndex, Key key, PageId childId, bool rightChild) {
    if (PageGet(entryId)->type == kLeaf) {
        BPlusEntry* leaf = PageGet(entryId);
        for (int j = leaf->count - 1; j >= keyIndex; j--) {
            GetLeafInternalEntry(leaf, j + 1)->key = GetLeafInternalEntry(leaf, j)->key;
        }
        GetLeafInternalEntry(leaf, keyIndex)->key = key;
        leaf->count++;
    }
    else {
        BPlusEntry* index = PageGet(entryId);
        for (int j = index->count - 1; j >= keyIndex; j--) {
            GetIndexInternalEntry(index, j + 1)->key = GetIndexInternalEntry(index, j)->key;
            GetIndexInternalEntry(index, j + 2)->childId = GetIndexInternalEntry(index, j + 1)->childId;
        }
        GetIndexInternalEntry(index, keyIndex)->key = key;
        PageGet(childId)->parentId = entryId;
        if (!rightChild) {
            // 是右子节点，多挪一个，并 keyIndex-- 使其指向右子节点
            GetIndexInternalEntry(index, keyIndex + 1)->childId = GetIndexInternalEntry(index, keyIndex)->childId;
            keyIndex--;
        }
        GetIndexInternalEntry(index, keyIndex + 1)->childId = childId;
        index->count++;
    }
}

/*
* 从内部节点数组中删除指定索引的内部节点
* 返回被删除内部节点的子节点
*/
static PageId BPlusDeleteInternalEntry(PageId entryId, int keyIndex, Key* key, bool rightChild) {
    if (PageGet(entryId)->type == kLeaf) {
        BPlusEntry* leaf = PageGet(entryId);
        if (key) {
            *key = GetLeafInternalEntry(leaf, keyIndex)->key;
        }
        for (int i = keyIndex + 1; i < leaf->count; i++) {
            GetLeafInternalEntry(leaf, i - 1)->key = GetLeafInternalEntry(leaf, i)->key;
        }
        leaf->count--;
        return NULL;
    }
    else {
        BPlusEntry* index = PageGet(entryId);
        if (key) {
            *key = GetIndexInternalEntry(index, keyIndex)->key;
        }
        PageId deleteEntryId;
        if (rightChild) {
            deleteEntryId = GetIndexInternalEntry(index, keyIndex + 1)->childId;
            for (int i = keyIndex + 1; i < index->count; i++) {
                GetIndexInternalEntry(index, i - 1)->key = GetIndexInternalEntry(index, i)->key;
                GetIndexInternalEntry(index, i)->childId = GetIndexInternalEntry(index, i + 1)->childId;
            }
        } else {
            deleteEntryId = GetIndexInternalEntry(index, keyIndex)->childId;
            for (int i = keyIndex + 1; i < index->count; i++) {
                GetIndexInternalEntry(index, i - 1)->key = GetIndexInternalEntry(index, i)->key;
                GetIndexInternalEntry(index, i - 1)->childId = GetIndexInternalEntry(index, i)->childId;
            }
            GetIndexInternalEntry(index, index->count - 1)->childId = GetIndexInternalEntry(index, index->count)->childId;
        }
        index->count--;
        return deleteEntryId;
    }
}

/*
* 分裂节点
* 同一个叶节点多次分裂会导致重复的key上升吗？
* 如果叶节点不存在相同的key，那么是不会的
* 因为分裂后选择右节点的最左内部节点的key作为上升的key
* 这个时候无论怎么插入内部节点都不会插入到该节点最左内部节点的左侧(比它小的会被分到左侧节点，因为父索引内部节点key等于该内部节点)，该节点再分裂也就不存在最左内部节点再次上升的可能了
*/
static bool BPlusInsertEntry(BPlusTree* tree, PageId entry, Key key, PageId rightChildId);
static void BPlusSplitEntry(BPlusTree* tree, PageId entryId, int insertIndex, Key key, PageId rightChildId) {
    BPlusEntry* entry = PageGet(entryId);
    BPlusEntry* rightChild = PageGet(rightChildId);

    PageId rightId;
    BPlusEntry* right;
    int rightCount;
    if (entry->type == kLeaf) {
        rightId = BPlusCreateLeafEntry(tree->m);
        right = PageGet(rightId);
        LeafListEntryInsertTail(entryId, rightId);
        // 原地分裂思路：mid将未插入的内部节点也算上，好计算newCount，4阶插入后4节点就是2(左2右2)，5阶插入后5节点还是2(左2右3)
        // 就是提前算好右侧应当有多少个内部节点，拷贝过去，中间遇到新内部节点插入就代替这一次的拷贝，没插入再插入到左侧
        int mid = tree->m / 2;
        rightCount = entry->count + 1 - mid;        // +1是因为这个时候entry->count并没有把未插入内部节点也算上
        int i = rightCount - 1, j = entry->count - 1;
        bool insert = false;
        for (; i >= 0; i--, j--) {
            if (!insert && j+1 == insertIndex) {        // 这里j+1是因为，循环的时候j并没有把未插入内部节点也算上
                entry->count++;
                GetLeafInternalEntry(right, i)->key = key;
                j++;        // j不动
                insert = true;
                continue;
            }
            GetLeafInternalEntry(right, i)->key = GetLeafInternalEntry(entry, j)->key;
        }

        entry->count -= rightCount;

        if (!insert) {
            // 新内部节点还没有插入，将其插入
            BPlusInsertInternalEntry(entryId, insertIndex, key, rightChildId, true);
        }
        
        // 从mid拿到上升内部节点
        key = GetLeafInternalEntry(right, 0)->key;
    }
    else {
        rightId = BPlusCreateIndexEntry(tree->m);
        right = PageGet(rightId);
        // 原地分裂思路：mid将未插入的内部节点和即将上升的内部节点都算上，好计算newCount，4阶插入后4节点就是4/2=2(左1右2)，5阶插入后5节点也是2(左2右2)，少了一个是因为上升的也算上了

        // 先将后半部分拷贝到新节点，如果中间遇到了索引的插入，那就一并插入，最后的midkey是entry->indexData[entry->count-1]，因为右侧的数量是提前算好的，多出来的一定放到左侧
        //  1 2       3 4
        // / |  |      |  \    
        // 此时新节点缺少了一条链接，我们最终选用旧节点的末尾内部节点作为上升内部节点，故旧节点的左侧的末尾内部节点的右子树就可以挂接到新节点上

        int mid = (tree->m-1) / 2;
        rightCount = entry->count - mid;        // 这个时候entry->count并没有把未插入内部节点也算上，但是会上升一个内部节点，抵消故不计入
        int i = rightCount - 1, j = entry->count - 1;
        bool insert = false;
        for (; i >= 0; i--, j--) {
            if (!insert && j+1 == insertIndex) {        // 这里j+1是因为，循环的时候j并没有把未插入内部节点也算上
                entry->count++;
                GetIndexInternalEntry(right, i)->key = key;
                GetIndexInternalEntry(right, i+1)->childId = rightChildId;
                rightChild->parentId = rightId;
                j++;        // j不动
                insert = true;
                continue;
            }
            GetIndexInternalEntry(right, i)->key = GetIndexInternalEntry(entry, j)->key;
            GetIndexInternalEntry(right, i+1)->childId = GetIndexInternalEntry(entry, j+1)->childId;
            PageGet(GetIndexInternalEntry(right, i + 1)->childId)->parentId = rightId;
        }
        entry->count -= rightCount;

        if (!insert) {
            // 新内部节点还没有插入，将其插入
            BPlusInsertInternalEntry(entryId, insertIndex, key, rightChildId, true);
        }

        // 最后从entry->indexData末尾拿到上升内部节点，将其摘除
        entry->count--;
        key = GetIndexInternalEntry(entry, entry->count)->key;

        // entry最右的链接交给newEntry
        GetIndexInternalEntry(right, 0)->childId = GetIndexInternalEntry(entry, entry->count + 1)->childId;
        PageGet(GetIndexInternalEntry(right, 0)->childId)->parentId = rightId;
    }
    right->count = rightCount;

    // 分裂出的内部节点向上传递
    if (entry->parentId == NULL) {
        PageId newIndexId = BPlusCreateIndexEntry(tree->m);
        entry->parentId = newIndexId;
        BPlusEntry* newIndex = PageGet(newIndexId);
        GetIndexInternalEntry(newIndex, 0)->childId = entryId;
        BPlusInsertInternalEntry(newIndexId, 0, key, rightId, true);
        tree->rootId = newIndexId;
    }
    else {
        BPlusInsertEntry(tree, entry->parentId, key, rightId);
    }
}

/*
* 插入节点
*/
static bool BPlusInsertEntry(BPlusTree* tree, PageId entryId, Key key, PageId rightChildId) {
    BPlusEntry* entry = PageGet(entryId);
    if (entry->count == 0) {
        BPlusInsertInternalEntry(entryId, 0, key, rightChildId, true);
        return true;
    }
    int insertIndex;
    int res;
    if (entry->type == kLeaf) {
        insertIndex = BinarySearchByField_Range_Custom(entry->leafInternalEntry, BPlusLeafInternalEntry, key, 0, entry->count - 1, &key, MemoryCmpR);
        res = MemoryCmpR(&GetLeafInternalEntry(entry, insertIndex)->key, &key, sizeof(key));
    }
    else {
        insertIndex = BinarySearchByField_Range_Custom(entry->indexInternalEntry, BPlusIndexInternalEntry, key, 0, entry->count - 1, &key, MemoryCmpR);
        res = MemoryCmpR(&GetIndexInternalEntry(entry, insertIndex)->key, &key, sizeof(key));
    }
    //if (res == 0) {        // 允许插入相同的节点
    //    return false;
    //}
    if (res < 0) {
        insertIndex++;        // 如果所有节点都小于key，那就追加到最尾部
    }

    if (entry->count < tree->m - 1) {
        // 有空余的位置插入
        BPlusInsertInternalEntry(entryId, insertIndex, key, rightChildId, true);
        return true;
    }

    // 没有多余位置，需要分裂向上合并
    BPlusSplitEntry(tree, entryId, insertIndex, key, rightChildId);
    return true;
}

/*
* 合并节点
*   2
* 1   3
* 父、左、右的关系如下
*/
static bool BPlusDeleteEntry(BPlusTree* tree, PageId entryId, int deleteIndex, Array* stack);
static void BPlusMergeEntry(BPlusTree* tree, PageId leftId, PageId rightId, int commonParentIndex, Array* stack) {
    BPlusEntry* left = PageGet(leftId);
    BPlusEntry* right = PageGet(rightId);

    if (left->type == kLeaf) {
        LeafListEntryRemoveTail(leftId);
        // 是叶子节点，将right并入left中，并删除right的父索引内部节点
        for (int i = 0; i < right->count; i++) {
            GetLeafInternalEntry(left, left->count++)->key = GetLeafInternalEntry(right, i)->key;
        }
    } else {
        // 是索引节点，将即将被删除的父索引内部节点(子节点丢弃，因为父索引子节点就指向左和右)和right都并入到left中，删除right的父索引内部节点
        GetIndexInternalEntry(left, left->count++)->key = GetIndexInternalEntry(PageGet(right->parentId), commonParentIndex)->key;
        // 右节点会多出一个头部子节点，因为父节点不需要带子节点下降，所以可以直接并到左节点尾部
        
        GetIndexInternalEntry(left, left->count)->childId = GetIndexInternalEntry(right, 0)->childId;
        PageGet(GetIndexInternalEntry(right, 0)->childId)->parentId = leftId;
        for (int i = 0; i < right->count; i++) {
            GetIndexInternalEntry(left, left->count++)->key = GetIndexInternalEntry(right, i)->key;
            GetIndexInternalEntry(left, left->count)->childId = GetIndexInternalEntry(right, i+1)->childId;
            PageGet(GetIndexInternalEntry(right, i+1)->childId)->parentId = leftId;
        }
    }
    // 合并就是从上面拿内部节点，已经拿了，要把删除也处理一下
    BPlusDeleteEntry(tree, left->parentId, commonParentIndex, stack);
    PageFree(rightId);
}

/*
* 删除指定节点
*/
static bool BPlusDeleteEntry(BPlusTree* tree, PageId entryId, int deleteIndex, Array* stack) {
    BPlusEntry* entry = PageGet(entryId);

    BPlusDeleteInternalEntry(entryId, deleteIndex, NULL, true);       // 直接删除，不考虑子节点，因为索引节点的删除是从底下传递上来的，底下已经处理好了被删除内部节点的子节点关系了，而叶子节点又没有子节点。
    if (entry->count >= (tree->m-1) / 2) {
        return true;
    }

    int* leftParentIndexPtr = (int*)ArrayPopTail(stack);
    if (!leftParentIndexPtr) {
        // 没有父节点就已经到根节点了，是叶子节点就跳过，是索引节点则判断是否没有任何子节点了，是则变更余下最后一个子节点为根节点，否则直接结束
        if (entry->type == kIndex && entry->count == 0) {
            PageId childId = GetIndexInternalEntry(entry, 0)->childId;
            BPlusEntry* child = PageGet(childId);
            child->parentId = NULL;
            tree->rootId = childId;
            PageFree(entryId);
        }
        return true;
    }
    int leftParentIndex = *leftParentIndexPtr;

    PageId siblingId = BPlusGetLeftSiblingEntry(tree, entryId, leftParentIndex);
    bool leftSibling = true;
    if (!siblingId) {
        siblingId = BPlusGetRightSiblingEntry(tree, entryId, leftParentIndex);
        leftSibling = false;
    }
    BPlusEntry* sibling = PageGet(siblingId);
    if (sibling->count > (tree->m-1) / 2) {
        // 向兄弟借节点
        Key key;
        if (entry->type == kLeaf) {
            // 叶子节点处理较简单，可以直接移动
            if (leftSibling) {
                // 从左兄弟节点的末尾的内部节点插入到当前节点的头部并更新父内部节点key为借来的key
                PageId childId = BPlusDeleteInternalEntry(siblingId, sibling->count - 1, &key, true);
                BPlusInsertInternalEntry(entryId, 0, key, childId, true);
            }
            else {
                // 从右兄弟节点的头部的内部节点插入到当前节点的尾部并更新父内部节点key为右兄弟的新首内部节点
                PageId childId = BPlusDeleteInternalEntry(siblingId, 0, &key, true);
                BPlusInsertInternalEntry(entryId, entry->count, key, childId, true);
                key = GetIndexInternalEntry(sibling, 0)->key;         // 右节点的头内部节点key可能正好和共同父节点相等(此时和索引相等的key跑到左边，就会导致找不到)，因此key更新为新的首内部节点是最好的
                ++leftParentIndex;        // 当前节点是左节点，要找与兄弟节点的共同父节点来更新，所以++
            }
        }
        else {
            // 索引节点处理较复杂，需要下降父节点中当前节点和兄弟节点的共同父内部节点，上升兄弟的内部节点到共同父内部节点(即交换)
            if (leftSibling) {
                // 左兄弟节点的末尾内部节点上升到父节点的头部，父节点的对应内部节点下降到当前节点的头部，上升内部节点其右子节点挂在下降的父节点内部节点的左侧
                PageId rightChildId = BPlusDeleteInternalEntry(siblingId, sibling->count - 1, &key, true);
                BPlusInsertInternalEntry(entryId, 0, GetIndexInternalEntry(PageGet(entry->parentId), leftParentIndex)->key, rightChildId, false);
            } else {
                // 右兄弟节点的头内部节点上升到父节点的头部，父节点的对应内部节点下降到当前节点的尾部，上升内部节点其左子节点挂在下降的父节点内部节点的右侧
                PageId leftChildId = BPlusDeleteInternalEntry(siblingId, 0, &key, false);
                leftParentIndex++;        // 当前节点是左节点，要找与兄弟节点的共同父节点来更新，所以++
                BPlusInsertInternalEntry(entryId, entry->count, GetIndexInternalEntry(PageGet(entry->parentId), leftParentIndex)->key, leftChildId, true);
            }
        }
        GetIndexInternalEntry(PageGet(entry->parentId), leftParentIndex)->key = key;
        return true;
    }

    // 兄弟节点不够借，需要合并(合并了也不会超过m-1，因为一边不足m-1的一半，一边是m-1的一半，是索引节点合并也足够下降一个父内部节点)
    if (leftSibling) {
        BPlusMergeEntry(tree, siblingId, entryId, leftParentIndex, stack);
    } else {
        BPlusMergeEntry(tree, entryId, siblingId, leftParentIndex + 1, stack);        // 要求共同父索引
    }
    return true;
}

/*
* 根据key查找到指定的叶子节点
* stack返回每一层节点的父节点内部节点(左父)索引
*/
static PageId BPlusTreeFindLeaf(BPlusTree* tree, int key, Array* stack) {
    PageId id = tree->rootId;
    BPlusEntry* cur = PageGet(id);
    while (cur && cur->type == kIndex) {
        //       4        8         12
        //    /       |        |       \
        // 3  4      6  7      10        15
        int i = BinarySearchByField_Range_Custom(cur->indexInternalEntry, BPlusIndexInternalEntry, key, 0, cur->count - 1, &key, MemoryCmpR);
        int res = MemoryCmpR(&GetIndexInternalEntry(cur, i)->key, &key, sizeof(key));

        if (res > 0) {
            // 查找key较小，向左找，cur是i的左子节点，i是cur的右父索引
            id = GetIndexInternalEntry(cur, i)->childId;
            cur = PageGet(id);
        } else {
            // 查找key较大，向右找，cur是i的右子节点，i是cur的左父索引
            // 相等的key也向右边找，因为分裂的索引是右叶子的第一个节点上升的
            id = GetIndexInternalEntry(cur, ++i)->childId;
            cur = PageGet(id);
        }
        if (stack) {
            i--;        // 修正入栈的i为左父索引
            ArrayPushTail(stack, &i);
        }
    }
    return id;
}



void BPlusTreeInit(BPlusTree* tree, int m) {
    if (m < 3) {
        m = 3;      // 最少3阶，否则无法正常分裂
    }
    tree->rootId = BPlusCreateLeafEntry(m);
    tree->leafListFirst = tree->rootId;
    LeafListEntryInit(tree->leafListFirst);
    tree->m = m;
}

bool BPlusTreeFind(BPlusTree* tree, Key key) {
    BPlusEntry* leaf = PageGet(BPlusTreeFindLeaf(tree, key, NULL));
    int index = BinarySearchByField_Custom(leaf->leafInternalEntry, BPlusLeafInternalEntry, key, 0, leaf->count - 1, &key, MemoryCmpR);
    if (index == -1) {
        return false;
    }
    return true;
}

bool BPlusTreeInsert(BPlusTree* tree, Key key) {
    return BPlusInsertEntry(tree, BPlusTreeFindLeaf(tree, key, NULL), key, NULL);
}

bool BPlusTreeDelete(BPlusTree* tree, Key key) {
    Array stack;
    ArrayInit(&stack, 16, sizeof(int));
    PageId leafId = BPlusTreeFindLeaf(tree, key, &stack);
    BPlusEntry* leaf = PageGet(leafId);
    int deleteIndex = BinarySearchByField_Custom(leaf->leafInternalEntry, BPlusLeafInternalEntry, key, 0, leaf->count - 1, &key, MemoryCmpR);
    if (deleteIndex == -1) {
        ArrayRelease(&stack);
        return false;
    }
    bool success = BPlusDeleteEntry(tree, leafId, deleteIndex, &stack);

    ArrayRelease(&stack);
    return success;
}
