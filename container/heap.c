#include "heap.h"

static void ShiftUp(Heap* heap, uint32_t now) {
	Vector* vec = &heap->vector;
	uint32_t next = now / 2;
	
	while (next > 0) {
		void** objParent = VectorAt(vec, next, void);
		void* keyParent = GetFieldByFieldOffset(*objParent, heap->keyFieldOffset, void);
		void** objChild = VectorAt(vec, now, void);
		void* keyChild = GetFieldByFieldOffset(*objChild, heap->keyFieldOffset, void);
		
		// ���״��ڵ��ں��Ӿͽ���
		if (heap->cmpFunc(keyParent, keyChild , heap->keyFieldSize) >= 0) {
			break;
		}
		void* temp = *objParent;
		*objParent = *objChild;
		*objChild = temp;
		now = next;
		next = now / 2;
	}
}

static void ShiftDown(Heap* heap, uint32_t now) {
	Vector* vec = &heap->vector;
	uint32_t next = now * 2;
	while (next < VectorGetCount(&heap->vector)) {
		void** objParent = VectorAt(vec, now, void);
		void* keyParent = GetFieldByFieldOffset(*objParent, heap->keyFieldOffset, void);
		void** objChild = VectorAt(vec, next, void);
		void* keyChild = GetFieldByFieldOffset(*objChild, heap->keyFieldOffset, void);

		// �������Ӷ�С�ڵ��ڸ��׾ͽ���
		if (heap->cmpFunc(keyChild, keyParent, heap->keyFieldSize) <= 0) {
			next++;
			objChild = VectorAt(vec, next, void);
			keyChild = GetFieldByFieldOffset(*objChild, heap->keyFieldOffset, void);
			if (heap->cmpFunc(keyChild, keyParent, heap->keyFieldSize) <= 0) {
				break;
			}
		}
		void* temp = *objParent;
		*objParent = *objChild;
		*objChild = temp;
		now = next;
		next = now * 2;
	}
}

void Heapify() {

}

void HeapInit(Heap* heap, uint32_t high, uint32_t keyFieldOffset, uint32_t keyFieldSize, CmpFunc cmpFunc) {
	size_t capacity = 1;
	for (uint32_t i = 0; i < high; i++) {
		capacity *= 2;
	}
	VectorInit(&heap->vector, capacity);
	VectorPushTail(&heap->vector, NULL);		// ��������±��1��ʼ

	heap->keyFieldOffset = keyFieldOffset;
	heap->keyFieldSize = keyFieldSize;

	if (cmpFunc == NULL) {
		cmpFunc = MemoryCmpR;
	}
	heap->cmpFunc = cmpFunc;
}

void HeapRelease(Heap* heap, bool deleteObj) {
	VectorRelease(&heap->vector, deleteObj);
}

void HeapInsert(Heap* heap, void* obj) {
	uint32_t tailIndex = VectorPushTail(&heap->vector, obj);
	ShiftUp(heap, tailIndex);
}

void* HeapGetTop(Heap* heap) {
	if (VectorGetCount(&heap->vector) <= 1) {
		return NULL;
	}
	return *VectorAt(&heap->vector, 1, void);
}

void* HeapPopTop(Heap* heap) {
	uint32_t tailIndex = VectorGetCount(&heap->vector) - 1;
	if (tailIndex == 0) {
		return NULL;
	}
	void* topObj = *VectorAt(&heap->vector, 1, void);
	void* tailObj = VectorPopTail(&heap->vector);
	*VectorAt(&heap->vector, 1, void) = tailObj;
	ShiftDown(heap, 1);
	return topObj;
}
