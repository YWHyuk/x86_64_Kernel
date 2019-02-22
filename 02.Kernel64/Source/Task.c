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
//태스크 관련 함수-초기화
void kInitializeTCBPool( void ){
	int i; // 4바이트 초과 불가!
	//TCBPOOlMANAER 구조체 초기화
	gs_stTCBPoolManager.pstTCBStartAddress = (TCB*) TASK_TCBPOOLADDRESS;
	gs_stTCBPoolManager.iTaskCount = 0;
	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.dwAllocatedCount = 1;
	//
	kMemCpy(TASK_TCBPOOLADDRESS, 0, sizeof(TCB)*TASK_MAXCOUNT);
	//TCB POOL 영역 초기화
	for(i=0;i<TASK_MAXCOUNT;i++){
		InitializeLinkedList(&(gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList));
		gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList.Node_ID=(QWORD)i;
	}
	//STACK POOL 초기화? 할 필요있나?
}
//태스크 관련 함수-할당 해제 *사실상 qwID만 할당한다고 보는게 더 정확*
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
//태스크 관련 함수-생성
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
	if(kAddTaskToReadyList(pstNewTCB)==FALSE){//TCB와 링크드 리스트는 한몸이므로, 둘중 하나만 걸릴 수 없다.
		kFreeTCB(pstNewTCB->stLinkedList.Node_ID);			//여기로 들어가면 이상한 일이 발생한 것..
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
	//새로운 태스크는 기본적으로 인터럽트가 활성화 되어있다!
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;
}
void kInitializeScheduler( void ){
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	gs_stScheduler.pstRunningTCB = kAllocTCB();
	InitializeLinkedListManger(&(gs_stScheduler.stReadyList));
}
//스케줄러 관련 함수-TCB* pstCurrentTCB 접근 함수;
void kSetRunningTCB( TCB* pstTask){
	gs_stScheduler.pstRunningTCB = pstTask;
}
TCB* kGetRunningTCB( void ){
	return gs_stScheduler.pstRunningTCB;
}
//스케줄러 관련 함수-LIST stReadyList 접근 함수;
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
//스케줄러 관련 함수-iProcessorTime 겁근 함수
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
//스케줄러 함수
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
	//컨택스트 스위칭 핵심 부분
	kContextSwitch(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	/* 헷갈리던 부분
	 * 만약 새로운 태스크의 컨택스트로 바뀐다면, RFLAGS 또한 바뀌므로 다시 인터럽트가 활성화 될 수 있다.
	 * 만약 새로운 태스크가 아니라, 저장된 태스크라면 전에 저장된 RFLAGS를 복원하므로 인터럽트가 다시 활성화 될 수 있다.
	 * 새로운 태스크를 만드는 곳에 인터럽트 활성화 비트가 체크되어있는지 확인해 보자.
	 */

	kSetInterruptFlag(bPreviousFlag);
}
BOOL kScheduleInterrupt( void ){
	/*
	 * 인터럽트에서 발생되는 부분이므로 인터럽트가 재발생할 걱정은 안해도 되는 건가?
	 * 인터럽트에서 context switching을 하려면, 단순히 커널 스택에 저장된 CONTEXT를 다음 태스크의 CONTEXT로 바꾸면 된다.
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
	//컨택스트 스위칭 핵심 부분
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
