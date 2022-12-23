#ifndef UTILS_QUEUE_H_
#define UTILS_QUEUE_H_

#include "CUtils/container/object.h"
#include "CUtils/container/array.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ArrayQueue {
	Array arr;
	int head;		// headָ�򼴽����ӵ�Ԫ������
	int tail;		// tailָ�������ӵ�Ԫ����������һ������
} ArrayQueue;

//        t=h
// �ӿգ�|   |   |   |
//             t   h
// ������| 3 |   | 1 | 2 |

void ArrayQueueExpand(ArrayQueue* queue);

void ArrayQueueInit(ArrayQueue* queue, size_t capacity, int objByteCount);

void ArrayQueueRelease(ArrayQueue* queue);

bool ArrayQueueIsEmpty(ArrayQueue* queue);

bool ArrayQueueIsFull(ArrayQueue* queue);

void ArrayQueueEnqueue(ArrayQueue* queue, void* obj);

void* ArrayQueueDequeue(ArrayQueue* queue);


#ifdef __cplusplus
}
#endif

#endif // UTILS_VECTOR_H_