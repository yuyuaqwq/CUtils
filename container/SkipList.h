#ifndef CUTILS_CONTAINER_SKIP_LIST_H_
#define CUTILS_CONTAINER_SKIP_LIST_H_

#include "CUtils/object.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
* ���������ڵ�
*/
typedef struct _SkipListLevel {
	struct _SkipListEntry* next;		// ͬ���������һ�ڵ�
} SkipListLevel;

/*
* ����ڵ㣬ÿ���ڵ�������������������������������鶯̬����
*/
typedef struct _SkipListEntry {
	void* obj;		// �޸�Ϊ�洢obj
	SkipListLevel upper[];		// �ڵ���ϲ㣬�������ڵ�
} SkipListEntry;

typedef struct _SkipList {
	SkipListEntry* head;
	int level;

	int keyFieldOffset;
	int keyFieldSize;
	CmpFunc cmpFunc;
} SkipList;


void SkipListInit(SkipList* list, int keyFieldOffset, int keyFieldSize, CmpFunc cmpFunc);
#define SkipListInitM(list, objName, keyFieldName) SkipListInit((list), GetFieldOffset(objName, keyFieldName), GetFieldSize(objName, keyFieldName), NULL)
void SkipListRelease(SkipList* list, bool deleteObj);
void* SkipListFind(SkipList* list, void* key);
bool SkipListInsert(SkipList* list, void* obj);
void* SkipListDelete(SkipList* list, void* key);
SkipListEntry* SkipListFirst(SkipList* list);
SkipListEntry* SkipListNext(SkipList* list, SkipListEntry* cur);

#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_STATIC_LIST_H_