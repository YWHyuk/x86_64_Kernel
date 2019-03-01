/*
 * Task.c
 *
 *  Created on: 2019. 2. 19.
 *      Author: DEWH
 */
#include "Task.h"
#include "Descriptor.h"
#include "Synchronization.h"

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
static TCB* kAllocTCB( void ){
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
static void kFreeTCB( QWORD qwID ){
	int i;
	i = GETTCBOFFSET(qwID);
	kMemCpy(&(gs_stTCBPoolManager.pstTCBStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstTCBStartAddress[i].stLinkedList.Node_ID = (long)i;

	gs_stTCBPoolManager.iTaskCount--;
}
//태스크 관련 함수-생성
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress,QWORD qwMemorySize, QWORD qwEntryPointAddress){
	TCB* pstProcessTCB;
	TCB* pstNewTCB;
	void* pvStackAddress;
	BOOL bPreviousFlag;
	//Critical section start-TCB_NODE_ID
	bPreviousFlag= kLockForSystemData();

	pstNewTCB = kAllocTCB();
	if(pstNewTCB == NULL)
		goto KALLOCTCBERROR;

	//태스크 영역의 메모리 정보 기입, 부모 프로세스 ID 기입
	pstProcessTCB = kGetProcessByThread(kGetRunningTCB());
	if(pstProcessTCB == NULL)
		goto KGETPROCESSBYTHREADERROR;

	if(qwFlags & TASK_FLAGS_THREAD){
		//태스크 영역의 쓰레드 생성 부분 처리
		pstNewTCB->qwParentTaskID = pstProcessTCB->stLinkedList.Node_ID;
		pstNewTCB->pvMemoryAddress = pstProcessTCB->pvMemoryAddress;
		pstNewTCB->qwMemorySize = pstProcessTCB->qwMemorySize;
		push_back(&(pstProcessTCB->stChildThreadList), &(pstNewTCB->stThreadLinkedList));
	}else{
		//태스크 영역의 프로세스 생성 부분 처리
		pstNewTCB->qwParentTaskID = pstNewTCB->stLinkedList.Node_ID;
		pstNewTCB->pvMemoryAddress = pvMemoryAddress;
		pstNewTCB->qwMemorySize = qwMemorySize;
	}
	pstNewTCB->stThreadLinkedList.Node_ID = pstNewTCB->stLinkedList.Node_ID;
	kUnLockForSystemData(bPreviousFlag);
	//Critical section end-TCB_NODE_ID

	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + \
			(GETTCBOFFSET(pstNewTCB->stLinkedList.Node_ID) * TASK_STACKSIZE));
	kSetUpTask(pstNewTCB, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

	InitializeLinkedListManger(&(pstNewTCB->stChildThreadList));
	//Critical section start-Writing TCB to Scheduler's Task List
	bPreviousFlag= kLockForSystemData();
	if(kAddTaskToReadyList(pstNewTCB)==FALSE)//TCB와 링크드 리스트는 한몸이므로, 둘중 하나만 걸릴 수 없다.
		goto KADDTASKTOREADYLISTERROR;			//여기로 들어가면 이상한 일이 발생한 것..

	kUnLockForSystemData(bPreviousFlag);
	//Critical section end-Writing TCB to Scheduler's Task List
	return pstNewTCB;

//ERROR HANDLING--------------------------------------------------------------------------
KGETPROCESSBYTHREADERROR:
	kPrintf("kGetProcessByThread() Failed...\n");
	goto KFREETCB;
KADDTASKTOREADYLISTERROR:
	kPrintf("kCreateTask() Failed...\n");
	goto KFREETCB;
KALLOCTCBERROR:
	goto KCOMMONERROR;
KFREETCB:
	kFreeTCB(pstNewTCB->stLinkedList.Node_ID);
KCOMMONERROR:
	kUnLockForSystemData(bPreviousFlag);
	return NULL;
}
static void kSetUpTask(TCB* pstTCB, QWORD qwFlags,QWORD qwEntryPointAddress ,\
		void* pvStackAddress, QWORD qwStackSize){
	kMemSet(&(pstTCB->stContext), 0, sizeof(CONTEXT));
	//pstTCB->stLinkedList
	pstTCB->qwFlags = qwFlags;
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

	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] =	(QWORD)pvStackAddress + qwStackSize - 8;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] =	(QWORD)pvStackAddress + qwStackSize - 8;
	*(QWORD*)((QWORD)pvStackAddress + qwStackSize - 8) = (QWORD) kExitTask;

	//interrupt flag set
	//새로운 태스크는 기본적으로 인터럽트가 활성화 되어있다!
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;
}
//태스크 관련 함수-우선순위 변경
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority){
	TCB* pstTargetTCB;
	BOOL bPreviousFlag;
	if(bPriority > TASK_MAXREADYLISTCOUNT){
		kPrintf("kChangePriority() passed wrong priority value...\n");
		return FALSE;
	}
	bPreviousFlag = kLockForSystemData();
	pstTargetTCB = gs_stScheduler.pstRunningTCB;
	if(pstTargetTCB->stLinkedList.Node_ID == qwTaskID){
		SETPRIORITY(pstTargetTCB->qwFlags, bPriority);

	}else{
		pstTargetTCB = kRemoveTaskReadyList(qwTaskID);
		if(pstTargetTCB == NULL){
			return FALSE;
		}else{
			SETPRIORITY(pstTargetTCB->qwFlags,bPriority);
			kAddTaskToReadyList(pstTargetTCB);
		}
	}
	kUnLockForSystemData(bPreviousFlag);
	return TRUE;

}
//태스크 관련 함수-태스크 종료
BOOL kEndTask(QWORD qwTaskID){
	TCB* pstTargetTCB = gs_stScheduler.pstRunningTCB;
	BOOL bPreviousFlag;
	//스스로 종료하는 입장?
	bPreviousFlag = kLockForSystemData();
	if(qwTaskID == pstTargetTCB->stLinkedList.Node_ID){
		//플래그 세우고
		pstTargetTCB->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTargetTCB->qwFlags , TASK_FLAGS_WAIT);
		//스케줄링
		kUnLockForSystemData(bPreviousFlag);
		kSchedule();
		kPrintf("kEndTaks() This message should not appear...\n");
		return TRUE;
	}else{
		//해당 태스크 플래그 세우고
		pstTargetTCB = kRemoveTaskReadyList(qwTaskID);
		if( pstTargetTCB == NULL){
			pstTargetTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
			if(pstTargetTCB != NULL){
				pstTargetTCB->qwFlags |= TASK_FLAGS_ENDTASK;
				SETPRIORITY(pstTargetTCB->qwFlags, TASK_FLAGS_WAIT);
			}
			kUnLockForSystemData(bPreviousFlag);
			return FALSE;
		}
		pstTargetTCB->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTargetTCB->qwFlags, TASK_FLAGS_WAIT);
		push_back(&(gs_stScheduler.stWaitList), pstTargetTCB);
		kUnLockForSystemData(bPreviousFlag);
		return TRUE;
	}
}
void kExitTask( void ){
	kEndTask(kGetRunningTCB()->stLinkedList.Node_ID);
}
//태스크 관련 함수-nodeId
TCB* kGetTCBInTCBPool(int iOffset){
	TCB* pstTCB;
	if(iOffset < -1 || iOffset >= TASK_MAXCOUNT)
		return NULL;
	pstTCB = (TCB*)TASK_TCBPOOLADDRESS;
	return &(pstTCB[iOffset]);
}
//쓰레드 관련 함수
TCB* kGetProcessByThread(TCB* pstThread){
	TCB* pstTCB;
	if(pstThread->qwFlags & TASK_FLAGS_PROCESS){
		return pstThread;
	}
	pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(pstThread->qwParentTaskID));
	if((pstTCB == NULL)|| (pstTCB->stLinkedList.Node_ID != pstThread->qwParentTaskID)){
		return NULL;
	}
	return pstTCB;
}
//스케줄러 관련 함수-초기화
void kInitializeScheduler( void ){
	int i;
	TCB* pstTCB;
	pstTCB = kAllocTCB();
	pstTCB->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_SYSTEM | TASK_FLAGS_PROCESS;
	pstTCB->qwParentTaskID = pstTCB->stLinkedList.Node_ID;
	pstTCB->pvMemoryAddress = (void*)0x100000;
	pstTCB->qwMemorySize = 0x500000;
	pstTCB->pvStackAddress = (void*)0x600000;
	pstTCB->qwStackSize = 0x100000;
	InitializeLinkedListManger(&(pstTCB->stChildThreadList));

	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	gs_stScheduler.pstRunningTCB = pstTCB;
	gs_stScheduler.qwProcessorLoad = 0;
	gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
	for(i=0;i<TASK_MAXREADYLISTCOUNT;i++){
		InitializeLinkedListManger(&(gs_stScheduler.stReadyList[i]));
		gs_stScheduler.viExecuteCount[i] = 0;
	}
	InitializeLinkedListManger(&(gs_stScheduler.stWaitList));
}
//스케줄러 관련 함수-TCB* pstCurrentTCB 접근 함수;
void kSetRunningTCB( TCB* pstTask){
	BOOL bPreviousFlag;
	bPreviousFlag = kLockForSystemData();
	gs_stScheduler.pstRunningTCB = pstTask;
	kUnLockForSystemData(bPreviousFlag);
}
TCB* kGetRunningTCB( void ){
	TCB* pstTCB;
	BOOL bPreviousFlag;
	bPreviousFlag = kLockForSystemData();
	pstTCB =gs_stScheduler.pstRunningTCB;
	kUnLockForSystemData(bPreviousFlag);
	return pstTCB;
}
//스케줄러 관련 함수-LIST stReadyList 접근 함수;
static TCB* kGetNextTaskToRun( void ){
	LINKEDLIST* pstLinkedList = NULL;
	LINKEDLISTMANAGER* pstLinkedListManager;
	int i;
	/*
	 * 스케줄러에서, 다음 태스크를 읽고 반환...
	 * */
	for(i = 0; i < TASK_MAXREADYLISTCOUNT * 2 ; i++){
		/*한 번 더 추가해준 이유는, 모든 리스트를 한번씩 통과할 수 있기때문.
		 *모든 리스트를 통과했다면, 각 리스트가 count가 0이거나,
		 *양보했기때문이다.
		 *만약 양보한 리스트 중 제일 빠른 것을 찾아야하기 때문에 한 번더 순회하자.
		 */
		pstLinkedListManager = &(gs_stScheduler.stReadyList[i % TASK_MAXREADYLISTCOUNT]);
		//만약 count가 0이라면 반드시 이 분기문을 지나야함...
		//실행 횟수는 최소 0이기 때문이다.
		if(count(pstLinkedListManager) <= gs_stScheduler.viExecuteCount[i % TASK_MAXREADYLISTCOUNT]){
			gs_stScheduler.viExecuteCount[i] = 0;
			continue;
		}
		pstLinkedList = pop_front(pstLinkedListManager);
		gs_stScheduler.viExecuteCount[i] +=1 ;
		break;
	}
	//kPrintf("kRemoveListFromHead() before...\n");
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	//kPrintf("kRemoveListFromHead() end...\n");
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	return (TCB*)pstLinkedList;
}
static BOOL kAddTaskToReadyList( TCB* pstTask){
	BYTE bPriority;
	bPriority = GETPRIORITY(pstTask->qwFlags);
	if(bPriority >= TASK_MAXREADYLISTCOUNT){
		return FALSE;
	}
	return push_back( &(gs_stScheduler.stReadyList[bPriority]), pstTask );
}
static TCB* kRemoveTaskReadyList( QWORD qwID ){
	BYTE bPriority;
	QWORD qwTCBIndex;
	TCB* pstTargetTCB;
	int iIndex;
	if(GETTCBOFFSET(qwID) >= TASK_MAXCOUNT){
		kPrintf("task ID is incorrect...\n");
		return NULL;
	}
	pstTargetTCB = &(((TCB*)TASK_TCBPOOLADDRESS)[GETTCBOFFSET(qwID)]);
	if(qwID != pstTargetTCB->stLinkedList.Node_ID){
		kPrintf("task ID is different with real task ID...\n");
		return NULL;
	}
	bPriority = GETPRIORITY(pstTargetTCB->qwFlags);
	iIndex = find(&(gs_stScheduler.stReadyList[bPriority]), qwID);
	if(iIndex == -1){
		kPrintf("qwID:[0x%q] find() failed...\n",qwID);
		return NULL;
	}
	return erase(&(gs_stScheduler.stReadyList[bPriority]), iIndex);
}
int kGetReadyTaskCount( void ){
	int iCount=0;
	int i;
	BOOL bPreviousFlag;
	bPreviousFlag = kLockForSystemData();
	for(i=0;i<TASK_MAXREADYLISTCOUNT;i++){
		iCount += count(&(gs_stScheduler.stReadyList[i]));
	}
	kUnLockForSystemData(bPreviousFlag);
	return iCount;
}
int kGetTaskCount( void ){
	int iCount;
	BOOL bPreviousFlag;
	iCount = kGetReadyTaskCount();
	bPreviousFlag = kLockForSystemData();
	iCount += count(&(gs_stScheduler.stWaitList)) + 1;
	kUnLockForSystemData(bPreviousFlag);

	return iCount;
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

	if(kGetReadyTaskCount() < 1){
		return;
	}

	bPreviousFlag = kLockForSystemData();
	pstNextTask = kGetNextTaskToRun();
	//kPrintf("%q: %q\n",pstNextTask,*pstNextTask);
	if(pstNextTask == NULL){
		kUnLockForSystemData(bPreviousFlag);
		return;
	}
	pstRunningTask = gs_stScheduler.pstRunningTCB;
	gs_stScheduler.pstRunningTCB = pstNextTask;
	//현재 태스크가 어떤 태스크인지 확인 후 각각 처리 과정을 달리하게 된다.
	//유휴 태스크 처리 분기문
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE)==TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += \
				(TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime);
	}
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	//종료를 기다리는 태스크 처리 분기문
	if((pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)==TASK_FLAGS_ENDTASK){
		if(push_back(&(gs_stScheduler.stWaitList), pstRunningTask)==FALSE){
			kUnLockForSystemData(bPreviousFlag);
			kPrintf("kAddTaskToReadyList() Failed... \n");
			return;
		}
		//컨택스트 스위칭 핵심 부분
		/* 헷갈리던 부분
		 * 만약 새로운 태스크의 컨택스트로 바뀐다면, RFLAGS 또한 바뀌므로 다시 인터럽트가 활성화 될 수 있다.
		 * 만약 새로운 태스크가 아니라, 저장된 태스크라면 전에 저장된 RFLAGS를 복원하므로 인터럽트가 다시 활성화 될 수 있다.
		 * 새로운 태스크를 만드는 곳에 인터럽트 활성화 비트가 체크되어있는지 확인해 보자.
		 */
		kContextSwitch(NULL, &(pstNextTask->stContext));
	}else{
		if(kAddTaskToReadyList(pstRunningTask)==FALSE){
			kUnLockForSystemData(bPreviousFlag);
			kPrintf("kAddTaskToReadyList() Failed... \n");
			return;
		}
		kContextSwitch(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	}
	kUnLockForSystemData(bPreviousFlag);
}
BOOL kScheduleInterrupt( void ){
	/*
	 * 인터럽트에서 발생되는 부분이므로 인터럽트가 재발생할 걱정은 안해도 되는 건가?
	 * 인터럽트에서 context switching을 하려면, 단순히 커널 스택에 저장된 CONTEXT를 다음 태스크의 CONTEXT로 바꾸면 된다.
	 *
	 */
	TCB* pstRunningTask, * pstNextTask;
	char* pstSavedContext;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	pstNextTask = kGetNextTaskToRun();
	if(pstNextTask == NULL){
		kUnLockForSystemData(bPreviousFlag);
		return FALSE;
	}
	pstSavedContext = (char*)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);
	pstRunningTask = gs_stScheduler.pstRunningTCB;
	gs_stScheduler.pstRunningTCB = pstNextTask;
	//유휴 태스크라면 시간 갱신
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE)==TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
	}
	//종료 대기중인 태스크
	//컨택스트 저장을 하지않고 불러오기만 한다.
	if((pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)==TASK_FLAGS_ENDTASK){
		push_back(&(gs_stScheduler.stWaitList), pstRunningTask);
	}
	//일반 태스크 처리
	else{
		kMemCpy(&(pstRunningTask->stContext),(void*)pstSavedContext,sizeof(CONTEXT));
		if(kAddTaskToReadyList(pstRunningTask)==FALSE){
			kPrintf("kAddTaskToReadyList() Failed... \n");
			kUnLockForSystemData(bPreviousFlag);
			return FALSE;
		}
	}
	//kPrintf("interrupt:%q\n",pstRunningTask);
	//kPrintf("interrupt:%q\n",pstNextTask);
	//컨택스트 스위칭 핵심 부분
	//kReadContext(&(pstNextTask->stContext));
	//kReadContext(pstSavedContext);
	//kReadContext(&(pstNextTask->stContext));
	//kMemCpy((void*)pstSavedContext,&(pstRunningTask->stContext),sizeof(CONTEXT));
	//((CONTEXT*)pstSavedContext)->vqRegister[TASK_RIPOFFSET]=\
			pstNextTask->stContext.vqRegister[TASK_RIPOFFSET];
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	kUnLockForSystemData(bPreviousFlag);
	kMemCpy((void*)pstSavedContext,&(pstNextTask->stContext), sizeof(CONTEXT));
	//kReadContext(pstSavedContext);
	return TRUE;

}

BOOL kIsTaskExist(QWORD qwID){
	TCB* pstTCB;
	pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));
	if(pstTCB == NULL || (pstTCB->stLinkedList.Node_ID !=qwID))
		return FALSE;
	return TRUE;
}
QWORD kGetProcessorLoad( void ){
	return gs_stScheduler.qwProcessorLoad;
}
void kIdleTask( void ){
	TCB* pstTask;
	TCB* pstChildThread;
	TCB* pstProcess;
	QWORD qwLastMeasureTickCount ,qwLastSpendTickInIdleTask;
	QWORD qwCurrentMeasureTickCount ,qwCurrentSpendTickInIdleTask;
	QWORD qwTaskID;
	BOOL bPreviousFlag;
	int iCount,i,iIndex;
	qwLastMeasureTickCount = kGetTickCount();
	qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
	while(1){
		qwCurrentMeasureTickCount = kGetTickCount();
		qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
		if((qwCurrentMeasureTickCount - qwLastMeasureTickCount) == 0){
			gs_stScheduler.qwProcessorLoad = 0;
		}
		else{
			gs_stScheduler.qwProcessorLoad = 100 - \
					(qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask) * 100 / \
					(qwCurrentMeasureTickCount - qwLastMeasureTickCount);
		}
		//kPrintf("LT:%q CT:%q LI:%q CI:%q\n",qwLastMeasureTickCount,qwCurrentMeasureTickCount,\
		//		qwLastSpendTickInIdleTask,qwCurrentSpendTickInIdleTask);
		qwLastMeasureTickCount = qwCurrentMeasureTickCount;
		qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;
		kHaltProcessorByLoad();

		if(count(&(gs_stScheduler.stWaitList)) > 0){
			while(1){
				bPreviousFlag = kLockForSystemData();
				pstTask = pop_front(&(gs_stScheduler.stWaitList));
				if(pstTask == NULL){
					kUnLockForSystemData(bPreviousFlag);
					break;
				}
				//프로세스라면?
				if(pstTask->qwFlags & TASK_FLAGS_PROCESS){
					iCount = count(&(pstTask->stChildThreadList));
					pstChildThread = front(&(pstTask->stChildThreadList));
					Print_LinkedList(&(pstTask->stChildThreadList));
					for( i = 0; i < iCount; i++ ){
						if(pstChildThread == NULL)
							break;
						//단순히 플래그를 세우고 유휴 태스크가 자식 쓰레드를 처리하길 기다린다.
						if(((GETTCBFROMTHREADLINK(pstChildThread))->qwFlags & TASK_FLAGS_ENDTASK) == 0)
							kEndTask((GETTCBFROMTHREADLINK(pstChildThread))->stLinkedList.Node_ID);
						pstChildThread = (TCB*)get_next_node((LINKEDLIST*)pstChildThread);
					}
					if(count(&(pstTask->stChildThreadList))>0){
						push_back(&(gs_stScheduler.stWaitList), pstTask);
						kUnLockForSystemData(bPreviousFlag);
						continue;
					}else{
						//memory clean
					}
				}else if(pstTask->qwFlags & TASK_FLAGS_THREAD){
					pstProcess = kGetProcessByThread(pstTask);
					if(pstProcess != NULL ){
						iIndex = find(&(pstProcess->stChildThreadList), pstTask->stLinkedList.Node_ID);
						if(iIndex == -1){
							kPrintf("Thread[0x%q] find() failed...\n",pstTask->stLinkedList.Node_ID);
							kUnLockForSystemData(bPreviousFlag);
							continue;
						}
						erase(&(pstProcess->stChildThreadList), iIndex);
					}
				}
				//태스크라면?
				qwTaskID = pstTask->stLinkedList.Node_ID;
				kFreeTCB(qwTaskID);
				kUnLockForSystemData(bPreviousFlag);
				kPrintf("IDLE: Task ID[0x%q] is completely ended.\n",\
										pstTask->stLinkedList.Node_ID);
			}
		}
		kSchedule();
	}
}
void kHaltProcessorByLoad( void ){
	if( gs_stScheduler.qwProcessorLoad < 40 ){
		kHlt();
		kHlt();
		kHlt();
	}else if( gs_stScheduler.qwProcessorLoad < 80 ){
		kHlt();
		kHlt();
	}else if( gs_stScheduler.qwProcessorLoad < 95 ){
		kHlt();
	}
}

//for debug
void kReadContext(CONTEXT* pstContext){
	int i;
	kPrintf("\n");
	for(i=0;i<TASK_REGISTERCOUNT;i++){
		kPrintf("%q:%q\t",&(pstContext->vqRegister[i]),pstContext->vqRegister[i]);
	}
	kPrintf("\n");
}
