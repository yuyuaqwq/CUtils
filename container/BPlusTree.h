#ifndef CUTILS_CONTAINER_BPLUS_TREE_H_
#define CUTILS_CONTAINER_BPLUS_TREE_H_

#include "CUtils/object.h"
#include "CUtils/algorithm/search.h"
#include "CUtils/container/list.h"

#ifdef __cplusplus
extern "C" {
#endif

// ��(m)��4��B��������4���ӽڵ㣬3���ڲ��ڵ㣬m = t * 2
// ��(t)���������ڵ��⣬ÿ���ڵ�������t���ڲ��ڵ�


typedef struct _BPlusLeafData {
	int key;		// void* obj;
} BPlusLeafData;

typedef struct _BPlusIndexData {		// Internal
	struct _BPlusEntry* child;
	int key;
} BPlusIndexData;

typedef enum {
	kIndex,
	kLeaf,
} BPlusEntryType;

// �����Ż��ĵ㣺child��key�ֱ�ϲ��ŵ�һ�𣬿��Լӿ�����Լ������ڴ����
typedef struct _BPlusEntry {
	BPlusEntryType type;
	int count;		// child / obj����
	struct _BPlusEntry* parent;
	union {
		BPlusIndexData indexData[];
		struct {
			ListEntry leafListEntry;
			BPlusLeafData leafData[];
		};
	};
} BPlusEntry;

typedef struct _BPlusTree {
	BPlusEntry* root;
	ListHead listHead;
	int m;
} BPlusTree;


static BPlusEntry* BPlusCreateIndexEntry(int m) {
	// child Ӧ����m����������������һ��
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) - sizeof(ListEntry) + (m - 1) * sizeof(BPlusIndexData) + sizeof(struct _BPlusIndexEntry*));
	entry->type = kIndex;
	for (int i = 0; i < m; i++) {
		entry->indexData[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}

static BPlusEntry* BPlusCreateLeafEntry(int m) {
	BPlusEntry* entry = (BPlusEntry*)MemoryAlloc(sizeof(BPlusEntry) + m * sizeof(BPlusLeafData));
	entry->type = kLeaf;
	// ListEntryInit(&entry->leafListEntry);
	for (int i = 0; i < m - 1; i++) {
		// entry->entry[i].child = NULL;
	}
	entry->count = 0;
	return entry;
}


static void BPlusInsertKeyToLeaf(BPlusEntry* leaf, int keyIndex, int key) {
	for (int j = leaf->count - 1; j >= keyIndex; j--) {
		leaf->leafData[j + 1].key = leaf->leafData[j].key;
	}
	leaf->leafData[keyIndex].key = key;
}

static void BPlusLeafSplit(BPlusTree* tree, BPlusEntry* leaf, int insertIndex, int key){
	int minT = tree->m / 2;
	int maxT = tree->m - minT;

	if (insertIndex < maxT) {

	}

	BPlusEntry* newLeaf = BPlusCreateLeafEntry(tree->m);

	
	// ������벿���ڲ��ڵ㵽��Ŀ
	for (int i = 0; i < t; i++) {
		newLeaf->leafData[i].key = leaf->leafData[t + i].key;
	}
	leaf->count -= t;
	newLeaf->count = t;
	
}

static BPlusEntry* BPlusTreeFindLeaf(BPlusTree* tree, int key) {
	BPlusEntry* cur = tree->root;
	while (cur && cur->type == kIndex) {
		//       4        8         12
		//    /       |        |       \
		// 3  4      6  7      10        15
		int i = BinarySearch_Range_CustomM(cur->indexData, BPlusIndexData, key, 0, cur->count - 1, &key, MemoryCmpR);
		if (cur->indexData[i].key <= key) {
			// ��ȵ�keyҲ�����
			cur = cur->indexData[i].child;
		}
		else {
			cur = cur->indexData[i + 1].child;
		}
	}
	return cur;
}


void BPlusTreeInit(BPlusTree* tree, int m) {
	if (m < 2) {
		m = 2;
	}
	tree->root = BPlusCreateLeafEntry(m);
	ListHeadInit(&tree->listHead);
	ListInsertHead(&tree->listHead, &tree->root->leafListEntry);
	tree->root->parent = NULL;
	tree->m = m;
}



bool BPlusTreeInsert(BPlusTree* tree, int key) {
	BPlusEntry* leaf = BPlusTreeFindLeaf(tree, key);

	if (leaf->count == 0) {
		BPlusInsertKeyToLeaf(leaf, 0, key);
		leaf->count++;
		return true;
	}

	int insertIndex = BinarySearch_Range_CustomM(leaf->leafData, BPlusLeafData, key, 0, leaf->count - 1, &key, MemoryCmpR);
	int res = MemoryCmpR(&leaf->leafData[insertIndex].key, &key, sizeof(key));
	if (res == 0) {
		return false;
	}
	if (leaf->count > 0 && res < 0) {
		insertIndex++;		// ������нڵ㶼С��key���Ǿ�׷�ӵ���β��
	}

	if (leaf->count < tree->m - 1) {
		// �п����λ�ò���
		BPlusInsertKeyToLeaf(leaf, insertIndex, key);
		leaf->count++;
		return true;
	}

	// û�ж���λ�ã���Ҫ�������Ϻϲ�
	BPlusLeafSplit(tree, leaf, insertIndex, key);

	return true;
}

#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_BPLUS_TREE_H_