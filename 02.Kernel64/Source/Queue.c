/*
 * Queue.c
 *
 *  Created on: 2019. 2. 16.
 *      Author: DEWH
 *
 * QUEUE :Manage struct
 * QUEUE.pvQueueArray: the place where the data is stored.
 *
 * pvQueue index check.
 *
 */
#include "Queue.h"
void kInitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize){
	pstQueue->iDataSize = iDataSize;
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->pvQueueArray = pvQueueBuffer;
	pstQueue->iPutIndex = 0;
	pstQueue->iGetIndex = 0;
	pstQueue->bLastOperationPut = QUEUE_GET;

}
BOOL kIsQueueFull( const QUEUE* pstQueue){
	/*������ �޼ҵ尡 put�̰� �Է°� ��� �ε����� ���ٸ� buffer Full*/
	if((pstQueue->iPutIndex==pstQueue->iGetIndex) && (pstQueue->bLastOperationPut==QUEUE_PUT))
		return TRUE;
	return FALSE;
}
BOOL kIsQueueEmpty( const QUEUE* pstQueue){
	/*������ �޼ҵ尡 get�̰� �Է°� ��� �ε����� ���ٸ� buffer Full*/
	if((pstQueue->iPutIndex==pstQueue->iGetIndex) && (pstQueue->bLastOperationPut==QUEUE_GET))
		return TRUE;
	return FALSE;
}
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData){
	//���� üũ..
	if(kIsQueueFull(pstQueue) == TRUE)
		return FALSE;

	int iDataSize= pstQueue->iDataSize;
	int iPutIndex= pstQueue->iPutIndex;

	kMemCpy((BYTE*)(pstQueue->pvQueueArray)+(iDataSize*iPutIndex),\
			pvData, iDataSize);
	pstQueue->iPutIndex =  (iPutIndex + 1)%(pstQueue->iMaxDataCount);
	pstQueue->bLastOperationPut = QUEUE_PUT;
	return TRUE;
}
BOOL kGetQueue(QUEUE* pstQueue, void* pvData){
	//���� üũ..
	if(kIsQueueEmpty(pstQueue) == TRUE)
		return FALSE;

	int iDataSize= pstQueue->iDataSize;
	int iGetIndex= pstQueue->iGetIndex;
	//����� ������ �κ�.. �޸𸮿� �߸��� ���� �� �� �ִ�.
	kMemCpy(pvData, (BYTE*)(pstQueue->pvQueueArray)+(iDataSize*iGetIndex),\
			iDataSize);
	pstQueue->iGetIndex =  (iGetIndex + 1)%(pstQueue->iMaxDataCount);
	pstQueue->bLastOperationPut = QUEUE_GET;
	return TRUE;
}

