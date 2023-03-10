/*
* @yuyuaqwq - 鱼鱼
* email:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* 请保留此声明
*/

#ifndef CUTILS_CONTAINER_ARRAY_H_
#define CUTILS_CONTAINER_ARRAY_H_

#include <CUtils/object.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _Array {
    size_t count;
    size_t capacity;
    void* objArr;
    int objSize;
} Array;

void ArrayInit(Array* arr, size_t capacity, int objByteCount);
void ArrayRelease(Array* arr);
#define ArrayAt(arr, index, objName) ((objName*)(((uintptr_t)(arr)->objArr) + (arr)->objSize * (index)))
#define ArrayToCArr(arr, objName) ((objName*)(arr)->objArr))
#define ArrayFindEntryByKeyField(arr, retObj, objName, keyFieldName, exp) { \
    retObj = NULL; \
    for (int i = 0; i < (arr)->count; i++) { \
        objName* tempObj = ArrayAt((arr), i, objName); \
        if (tempObj->##exp) { \
            retObj = tempObj; \
            break; \
        } \
    } \
}
int ArrayPushTail(Array* arr, void* obj);
void* ArrayPopTail(Array* arr);
void* ArrayGetData(Array* arr);
size_t ArrayGetCount(Array* arr);
void ArraySetCount(Array* arr, size_t count);
size_t ArrayGetByteCount(Array* arr);
size_t ArrayGetCapacity(Array* arr);
void ArraySetCapacity(Array* arr, size_t capacity);
void ArrayResetCapacity(Array* arr, size_t capacity);
int ArrayGetObjectSize(Array* arr);
int ArrayGetObjectSize(Array* arr);
void ArrayExpand(Array* arr, size_t addCount);
void ArraySwapEntry(Array* arr, int index1, int index2);

#ifdef __cplusplus
}
#endif

#endif // CUTILS_CONTAINER_ARRAY_H_