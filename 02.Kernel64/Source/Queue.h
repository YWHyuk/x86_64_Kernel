/*
 * Queue.h
 *
 *  Created on: 2019. 2. 16.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_QUEUE_H_
#define __02_KERNEL64_SOURCE_QUEUE_H_
#include "Types.h"
#pragma pack(push, 1)
#define QUEUE_PUT TRUE
#define QUEUE_GET FALSE
typedef struct kQueueManagerStruct{
	int iDataSize;
	int iMaxDataCount;
	void* pvQueueArray;
	int iPutIndex; // �Է� �ε��� ��ġ
	int iGetIndex; // ��� �ε��� ��ġ
	BOOL bLastOperationPut; //������ ȣ�� ���.
}QUEUE;
#pragma pack(pop)
void kInitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize);
BOOL kIsQueueFull( const QUEUE* pstQueue);
BOOL kIsQueueEmpty( const QUEUE* pstQueue);
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData);
BOOL kGetQueue(QUEUE* pstQueue, void* pvData);

#endif /* __02_KERNEL64_SOURCE_QUEUE_H_ */
