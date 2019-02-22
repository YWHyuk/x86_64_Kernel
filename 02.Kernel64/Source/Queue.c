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
	/*마지막 메소드가 put이고 입력과 출력 인덱스가 같다면 buffer Full*/
	if((pstQueue->iPutIndex==pstQueue->iGetIndex) && (pstQueue->bLastOperationPut==QUEUE_PUT))
		return TRUE;
	return FALSE;
}
BOOL kIsQueueEmpty( const QUEUE* pstQueue){
	/*마지막 메소드가 get이고 입력과 출력 인덱스가 같다면 buffer Full*/
	if((pstQueue->iPutIndex==pstQueue->iGetIndex) && (pstQueue->bLastOperationPut==QUEUE_GET))
		return TRUE;
	return FALSE;
}
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData){
	//버퍼 체크..
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
	//버퍼 체크..
	if(kIsQueueEmpty(pstQueue) == TRUE)
		return FALSE;

	int iDataSize= pstQueue->iDataSize;
	int iGetIndex= pstQueue->iGetIndex;
	//상당히 위험한 부분.. 메모리에 잘못된 값을 쓸 수 있다.
	kMemCpy(pvData, (BYTE*)(pstQueue->pvQueueArray)+(iDataSize*iGetIndex),\
			iDataSize);
	pstQueue->iGetIndex =  (iGetIndex + 1)%(pstQueue->iMaxDataCount);
	pstQueue->bLastOperationPut = QUEUE_GET;
	return TRUE;
}

