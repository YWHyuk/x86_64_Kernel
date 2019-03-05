/*
 * Synchronization.c
 *
 *  Created on: 2019. 2. 26.
 *      Author: DEWH
 */
#include "Synchronization.h"
#include "AssemblyUtility.h"
void kInitializeMutex(MUTEX* pstMutex){
	pstMutex->qwTaskID = TASK_INVALIDID;
	pstMutex->dwLockCount = 0;
	pstMutex->bLockFlag = 0;
}
void kLock(MUTEX* pstMutex){
	if(kTestAndSet(&(pstMutex->bLockFlag), 0, 1)==FALSE){
		if(pstMutex->qwTaskID == kGetRunningTCB()->stLinkedList.Node_ID){
				pstMutex->dwLockCount++;
				return;
		}
		while(kTestAndSet(&(pstMutex->bLockFlag), 0, 1)==FALSE){
			kSchedule();
			//kPrintf("task [0x%q] In Mutex: Im waiting..\n",kGetRunningTCB()->stLinkedList.Node_ID);
		}
	}
	pstMutex->qwTaskID = (kGetRunningTCB())->stLinkedList.Node_ID;
	pstMutex->dwLockCount= 1;
}
void kUnlock(MUTEX* pstMutex)
{
	if((pstMutex->bLockFlag == 0)|| \
			(pstMutex->qwTaskID != kGetRunningTCB()->stLinkedList.Node_ID))
	{
		return;
	}
	if(pstMutex->dwLockCount>1){
		pstMutex->dwLockCount -= 1;
	}else{
		pstMutex->qwTaskID = TASK_INVALIDID;
		pstMutex->dwLockCount = 0;
		pstMutex->bLockFlag = FALSE;//�� �ڵ带 ���������� mutal exclusion�� ����ȴ�.
	}
}
// ���ͷ�Ʈ �÷��� ���� �Լ�
BOOL kLockForSystemData( void ){
	return kSetInterruptFlag(FALSE);
}
void kUnLockForSystemData ( BOOL bInterruptFlag ){
	kSetInterruptFlag(bInterruptFlag);
}

