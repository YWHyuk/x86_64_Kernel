/*
 * Task.c
 *
 *  Created on: 2019. 2. 19.
 *      Author: DEWH
 */
#include "Task.h"
#include "Descriptor.h"

static TCBPOOLMANAGER gs_stTCBPoolManager;
static SCHEDULER gs_stScheduler;
//�½�ũ ���� �Լ�-�ʱ�ȭ
void kInitializeTCBPool( void ){
	int i; // 4����Ʈ �ʰ� �Ұ�!
	//TCBPOOlMANAER ����ü �ʱ�ȭ
	gs_stTCBPoolManager.pstTCBStartAddress = (TCB*) TASK_TCBPOOLADDRESS;
	gs_stTCBPoolManager.iTaskCount = 0;
	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.dwAllocatedCount = 1;
	//
	kMemCpy(TASK_TCBPOOLADDRESS, 0, sizeof(TCB)*TASK_MAXCOUNT);
	//TCB POOL ���� �ʱ�ȭ
	for(i=0;i<TASK_MAXCOUNT;i++){
		InitializeLinkedList(&(gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList));
		gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList.Node_ID=(QWORD)i;
	}
	//STACK POOL �ʱ�ȭ? �� �ʿ��ֳ�?
}
//�½�ũ ���� �Լ�-�Ҵ� ���� *��ǻ� qwID�� �Ҵ��Ѵٰ� ���°� �� ��Ȯ*
TCB* kAllocTCB( void ){
	TCB* pstTCB;
	if( gs_stTCBPoolManager.iTaskCount == gs_stTCBPoolManager.iMaxCount){
		goto ALLOCTCB_ERROR;
	}
	for(pstTCB = (TCB*)TASK_TCBPOOLADDRESS;pstTCB < (TCB*)(TASK_TCBPOOLADDRESS + sizeof(TCB)* TASK_MAXCOUNT); pstTCB++){
		if((pstTCB->stLinkedList.Node_ID >> 32) == 0){
			pstTCB->stLinkedList.Node_ID |= (((QWORD)(gs_stTCBPoolManager.dwAllocatedCount)) << 32);
			//kPrintf("kAllocTCB:%q %q\n",pstTCB,pstTCB->stLinkedList.qwID);
			gs_stTCBPoolManager.iTaskCount++;
			gs_stTCBPoolManager.dwAllocatedCount++;
			if(gs_stTCBPoolManager.dwAllocatedCount == 0){
				gs_stTCBPoolManager.dwAllocatedCount++;
			}
			return pstTCB;
		}
	}
ALLOCTCB_ERROR:
	kPrintf("TCB Pool is full...\n");
	return NULL;
}
void kFreeTCB( QWORD qwID ){
	int i;
	i = (int)(0xFFFFFFFFUL & qwID);
	kMemCpy(&(gs_stTCBPoolManager.pstTCBStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList.Node_ID = (long)i;

	gs_stTCBPoolManager.iTaskCount--;
}
//�½�ũ ���� �Լ�-����
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress){
	TCB* pstNewTCB;
	void* pvStackAddress;
	pstNewTCB = kAllocTCB();
	if(pstNewTCB == NULL){
		return NULL;
	}
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + \
			((pstNewTCB->stLinkedList.Node_ID & 0xFFFFFFFF) * TASK_STACKSIZE));
	kSetUpTask(pstNewTCB, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
	if(kAddTaskToReadyList(pstNewTCB)==FALSE){//TCB�� ��ũ�� ����Ʈ�� �Ѹ��̹Ƿ�, ���� �ϳ��� �ɸ� �� ����.
		kFreeTCB(pstNewTCB->stLinkedList.Node_ID);			//����� ���� �̻��� ���� �߻��� ��..
		return NULL;
	}
	//kPrintf("kCreatTask %q\n",pstNewTCB->stLinkedList.qwID);
	//kPrintf("head:%q tail%q\n",gs_stScheduler.stReadyList.pstHead->pvNext,gs_stScheduler.stReadyList.pstTail->pvNext);
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	return pstNewTCB;
}
void kSetUpTask(TCB* pstTCB, QWORD qwFlags,QWORD qwEntryPointAddress ,\
		void* pvStackAddress, QWORD qwStackSize){
	kMemSet(&(pstTCB->stContext), 0, sizeof(CONTEXT));
	//pstTCB->stLinkedList
	//pstTCB->qwFlags = qwFlags;
	pstTCB->qwStackSize = qwStackSize;
	pstTCB->pvStackAddress = pvStackAddress;
	//CONTEXT SegmentRegiser setting
	pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

	pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
	//RIP RSP RBP setting
	pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] =	(QWORD)pvStackAddress + qwStackSize;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] =	(QWORD)pvStackAddress + qwStackSize;

	//interrupt flag set
	//���ο� �½�ũ�� �⺻������ ���ͷ�Ʈ�� Ȱ��ȭ �Ǿ��ִ�!
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;
}
void kInitializeScheduler( void ){
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	gs_stScheduler.pstRunningTCB = kAllocTCB();
	InitializeLinkedListManger(&(gs_stScheduler.stReadyList));
}
//�����ٷ� ���� �Լ�-TCB* pstCurrentTCB ���� �Լ�;
void kSetRunningTCB( TCB* pstTask){
	gs_stScheduler.pstRunningTCB = pstTask;
}
TCB* kGetRunningTCB( void ){
	return gs_stScheduler.pstRunningTCB;
}
//�����ٷ� ���� �Լ�-LIST stReadyList ���� �Լ�;
TCB* kGetNextTaskToRun( void ){
	LINKEDLIST* pstLinkedList;
	if(count(&(gs_stScheduler.stReadyList)) == 0){
		//kPrintf("Task Ready List is Empty...\n");
		return NULL;
	}
	//kPrintf("kRemoveListFromHead() before...\n");
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	pstLinkedList = pop_front(&(gs_stScheduler.stReadyList));
	//kPrintf("kRemoveListFromHead() end...\n");
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	return (TCB*)pstLinkedList;
}
BOOL kAddTaskToReadyList( TCB* pstTask){
	return push_back(&(gs_stScheduler.stReadyList), pstTask);
}
//�����ٷ� ���� �Լ�-iProcessorTime �̱� �Լ�
void kDecreaseProcessorTime( void ){
	if(gs_stScheduler.iProcessorTime>0){
		gs_stScheduler.iProcessorTime--;
	}
}
BOOL kIsProcessorTimeExpired( void ){
	if(gs_stScheduler.iProcessorTime == 0){
		return TRUE;
	}
	return FALSE;
}
//�����ٷ� �Լ�
void kSchedule(void){
	TCB* pstRunningTask, * pstNextTask;
	BOOL bPreviousFlag;


	if(count(&(gs_stScheduler.stReadyList))==0){
		return;
	}
	bPreviousFlag = kSetInterruptFlag(FALSE);
	pstNextTask = kGetNextTaskToRun();
	//kPrintf("%q: %q\n",pstNextTask,*pstNextTask);
	if(pstNextTask == NULL){
		kSetInterruptFlag(bPreviousFlag);
		return;
	}
	pstRunningTask = gs_stScheduler.pstRunningTCB;
	if(kAddTaskToReadyList(pstRunningTask)==FALSE){
		kSetInterruptFlag(bPreviousFlag);
		return;
	}

	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	gs_stScheduler.pstRunningTCB = pstNextTask;
	//���ý�Ʈ ����Ī �ٽ� �κ�
	kContextSwitch(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	/* �򰥸��� �κ�
	 * ���� ���ο� �½�ũ�� ���ý�Ʈ�� �ٲ�ٸ�, RFLAGS ���� �ٲ�Ƿ� �ٽ� ���ͷ�Ʈ�� Ȱ��ȭ �� �� �ִ�.
	 * ���� ���ο� �½�ũ�� �ƴ϶�, ����� �½�ũ��� ���� ����� RFLAGS�� �����ϹǷ� ���ͷ�Ʈ�� �ٽ� Ȱ��ȭ �� �� �ִ�.
	 * ���ο� �½�ũ�� ����� ���� ���ͷ�Ʈ Ȱ��ȭ ��Ʈ�� üũ�Ǿ��ִ��� Ȯ���� ����.
	 */

	kSetInterruptFlag(bPreviousFlag);
}
BOOL kScheduleInterrupt( void ){
	/*
	 * ���ͷ�Ʈ���� �߻��Ǵ� �κ��̹Ƿ� ���ͷ�Ʈ�� ��߻��� ������ ���ص� �Ǵ� �ǰ�?
	 * ���ͷ�Ʈ���� context switching�� �Ϸ���, �ܼ��� Ŀ�� ���ÿ� ����� CONTEXT�� ���� �½�ũ�� CONTEXT�� �ٲٸ� �ȴ�.
	 *
	 */
	TCB* pstRunningTask, * pstNextTask;
	char* pstSavedContext;
	pstNextTask = kGetNextTaskToRun();
	if(pstNextTask == NULL){
		return FALSE;
	}


	pstSavedContext = (char*)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);
	pstRunningTask = gs_stScheduler.pstRunningTCB;
	if(kAddTaskToReadyList(pstRunningTask)==FALSE){
		return FALSE;
	}
	kMemCpy(&(pstRunningTask->stContext),(void*)pstSavedContext,sizeof(CONTEXT));

	gs_stScheduler.pstRunningTCB = pstNextTask;
	//kPrintf("interrupt:%q\n",pstRunningTask);
	//kPrintf("interrupt:%q\n",pstNextTask);
	//���ý�Ʈ ����Ī �ٽ� �κ�
	//kReadContext(&(pstNextTask->stContext));
	//kReadContext(pstSavedContext);
	//kReadContext(&(pstNextTask->stContext));
	//kMemCpy((void*)pstSavedContext,&(pstRunningTask->stContext),sizeof(CONTEXT));
	//((CONTEXT*)pstSavedContext)->vqRegister[TASK_RIPOFFSET]=\
			pstNextTask->stContext.vqRegister[TASK_RIPOFFSET];
	kMemCpy((void*)pstSavedContext,&(pstNextTask->stContext), sizeof(CONTEXT));
	//kReadContext(pstSavedContext);
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;

}
void kReadContext(CONTEXT* pstContext){
	int i;
	kPrintf("\n");
	for(i=0;i<TASK_REGISTERCOUNT;i++){
		kPrintf("%q:%q\t",&(pstContext->vqRegister[i]),pstContext->vqRegister[i]);
	}
	kPrintf("\n");
}
