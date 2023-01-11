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



void BPlusTreeInit(BPlusTree* tree, int m);
bool BPlusTreeInsert(BPlusTree* tree, int key);
bool BPlusTreeFind(BPlusTree* tree, int key);

#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_BPLUS_TREE_H_