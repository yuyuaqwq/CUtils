/*
* @yuyuaqwq - 鱼鱼
* email:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* 请保留此声明
*/

#include <CUtils/container/array.h>




void ArrayInit(Array* arr, size_t capacity, int objByteCount) {
    arr->capacity = 0;
    arr->count = 0;
    arr->objArr = NULL;
    arr->objSize = objByteCount;
    if (capacity != 0) {
        ArrayResetCapacity(arr, capacity);
    }
}

void ArrayRelease(Array* arr) {
    if (arr->objArr) {
        ObjectDelete(arr->objArr);
        arr->objArr = NULL;
    }
    arr->capacity = 0;
    arr->count = 0;
}

int ArrayPushTail(Array* arr, void* obj) {
    if (arr->capacity <= arr->count) {
        ArrayExpand(arr, 1);
    }
    MemoryCopy(ArrayAt(arr, arr->count++, void), obj, arr->objSize);
    return arr->count - 1;
}

void* ArrayPopTail(Array* arr) {
    if (arr->count == 0) {
        return NULL;
    }
    return ArrayAt(arr, --arr->count, void);
}

void ArraySwapEntry(Array* arr, int index1, int index2) {
    void* obj1 = ArrayAt(arr, index1, void);
    void* obj2 = ArrayAt(arr, index2, void);
    MemorySwap(obj1, obj2, arr->objSize);
}

void* ArrayGetData(Array* arr) {
    return arr->objArr;
}

size_t ArrayGetCount(Array* arr) {
    return arr->count;
}

void ArraySetCount(Array* arr, size_t count) {
    arr->count = count;
}

size_t ArrayGetByteCount(Array* arr) {
    return arr->count * arr->objSize;
}

size_t ArrayGetCapacity(Array* arr) {
    return arr->capacity;
}

void ArraySetCapacity(Array* arr, size_t capacity) {
    arr->capacity = capacity;
}

int ArrayGetObjectSize(Array* arr) {
    return arr->objSize;
}

void ArrayResetCapacity(Array* arr, size_t capacity) {
    void* newBuf = ObjectCreateArrayBySize(arr->objSize, capacity);
    if (arr->objArr) {
        MemoryCopy(newBuf, arr->objArr, arr->objSize * arr->count);
        ObjectDelete(arr->objArr);
    }
    arr->objArr = newBuf;
    arr->capacity = capacity;
}

void ArrayExpand(Array* arr, size_t addCount) {
    size_t curCapacity = arr->capacity;
    size_t targetCount = curCapacity + addCount;
    if (curCapacity == 0) {
        curCapacity = 1;
    }
    while (curCapacity < targetCount) {
        curCapacity *= 2;
    }
    ArrayResetCapacity(arr, curCapacity);
}