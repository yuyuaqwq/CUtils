#ifndef CUTILS_CONTAINER_STATIC_LIST_H_
#define CUTILS_CONTAINER_STATIC_LIST_H_

#include "CUtils/object.h"
#include "CUtils/container/array.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _StaticListHead {
	Array array;
	int entryFieldOffset;
	int freeIndex;
} StaticListHead;

/*
* ��������Ŀ��ֵ���Ա����ǣ��ʿ���ʹ��union
*/
typedef struct _StaticListEntry {
	int nextIndex;
} StaticListEntry;

void StaticListHeadInit(StaticListHead* head, size_t count, int objSize, int entryFieldOffset);
int StaticListAllocEntry(StaticListHead* head);
void StaticListFreeEntry(StaticListHead* head, int index);
#define StaticListAt(head, index, objName) (ArrayAt((head)->array, index, objName))
int StaticListSwitchFreeIndex(StaticListHead* head, int newIndex);

#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_STATIC_LIST_H_