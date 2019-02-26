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
//�½�ũ ���� �Լ�-����
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress){
	TCB* pstNewTCB;
	void* pvStackAddress;
	BOOL bPreviousFlag;

	bPreviousFlag= kLockForSystemData();
	pstNewTCB = kAllocTCB();
	if(pstNewTCB == NULL){
		kUnLockForSystemData(bPreviousFlag);
		return NULL;
	}
	kUnLockForSystemData(bPreviousFlag);
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + \
			(GETTCBOFFSET(pstNewTCB->stLinkedList.Node_ID) * TASK_STACKSIZE));
	kSetUpTask(pstNewTCB, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

	bPreviousFlag= kLockForSystemData();
	if(kAddTaskToReadyList(pstNewTCB)==FALSE){//TCB�� ��ũ�� ����Ʈ�� �Ѹ��̹Ƿ�, ���� �ϳ��� �ɸ� �� ����.
		kFreeTCB(pstNewTCB->stLinkedList.Node_ID);			//����� ���� �̻��� ���� �߻��� ��..

		kUnLockForSystemData(bPreviousFlag);
		kPrintf("kCreateTask() Failed...\n");
		return NULL;
	}
	kUnLockForSystemData(bPreviousFlag);
	//kPrintf("kCreatTask %q\n",pstNewTCB->stLinkedList.qwID);
	//kPrintf("head:%q tail%q\n",gs_stScheduler.stReadyList.pstHead->pvNext,gs_stScheduler.stReadyList.pstTail->pvNext);
	//kPrintLinkedList(&(gs_stScheduler.stReadyList));
	return pstNewTCB;
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

	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] =	(QWORD)pvStackAddress + qwStackSize;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] =	(QWORD)pvStackAddress + qwStackSize;

	//interrupt flag set
	//���ο� �½�ũ�� �⺻������ ���ͷ�Ʈ�� Ȱ��ȭ �Ǿ��ִ�!
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;
}
//�½�ũ ���� �Լ�-�켱���� ����
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
//�½�ũ ���� �Լ�-�½�ũ ����
BOOL kEndTask(QWORD qwTaskID){
	TCB* pstTargetTCB = gs_stScheduler.pstRunningTCB;
	BOOL bPreviousFlag;
	//������ �����ϴ� ����?
	bPreviousFlag = kLockForSystemData();
	if(qwTaskID == pstTargetTCB->stLinkedList.Node_ID){
		//�÷��� �����
		pstTargetTCB->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTargetTCB->qwFlags , TASK_FLAGS_WAIT);
		//�����ٸ�
		kUnLockForSystemData(bPreviousFlag);
		kSchedule();
		kPrintf("kEndTaks() This message should not appear...\n");
		return TRUE;
	}else{
		//�ش� �½�ũ �÷��� �����
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
//�½�ũ ���� �Լ�-nodeId
TCB* kGetTCBInTCBPool(int iOffset){
	TCB* pstTCB;
	if(iOffset < -1 || iOffset >= TASK_MAXCOUNT)
		return NULL;
	pstTCB = (TCB*)TASK_TCBPOOLADDRESS;
	return &(pstTCB[iOffset]);
}
//�����ٷ� ���� �Լ�-�ʱ�ȭ
void kInitializeScheduler( void ){
	int i;
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	gs_stScheduler.pstRunningTCB = kAllocTCB();
	gs_stScheduler.pstRunningTCB->qwFlags = TASK_FLAGS_HIGHEST;
	gs_stScheduler.qwProcessorLoad = 0;
	gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
	for(i=0;i<TASK_MAXREADYLISTCOUNT;i++){
		InitializeLinkedListManger(&(gs_stScheduler.stReadyList[i]));
		gs_stScheduler.viExecuteCount[i] = 0;
	}
	InitializeLinkedListManger(&(gs_stScheduler.stWaitList));
}
//�����ٷ� ���� �Լ�-TCB* pstCurrentTCB ���� �Լ�;
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
//�����ٷ� ���� �Լ�-LIST stReadyList ���� �Լ�;
static TCB* kGetNextTaskToRun( void ){
	LINKEDLIST* pstLinkedList = NULL;
	LINKEDLISTMANAGER* pstLinkedListManager;
	int i;
	/*
	 * �����ٷ�����, ���� �½�ũ�� �а� ��ȯ...
	 * */
	for(i = 0; i < TASK_MAXREADYLISTCOUNT * 2 ; i++){
		/*�� �� �� �߰����� ������, ��� ����Ʈ�� �ѹ��� ����� �� �ֱ⶧��.
		 *��� ����Ʈ�� ����ߴٸ�, �� ����Ʈ�� count�� 0�̰ų�,
		 *�纸�߱⶧���̴�.
		 *���� �纸�� ����Ʈ �� ���� ���� ���� ã�ƾ��ϱ� ������ �� ���� ��ȸ����.
		 */
		pstLinkedListManager = &(gs_stScheduler.stReadyList[i % TASK_MAXREADYLISTCOUNT]);
		//���� count�� 0�̶�� �ݵ�� �� �б⹮�� ��������...
		//���� Ƚ���� �ּ� 0�̱� �����̴�.
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
		kPrintf("find() failed...\n");
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
	//���� �½�ũ�� � �½�ũ���� Ȯ�� �� ���� ó�� ������ �޸��ϰ� �ȴ�.
	//���� �½�ũ ó�� �б⹮
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE)==TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += \
				(TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime);
	}
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	//���Ḧ ��ٸ��� �½�ũ ó�� �б⹮
	if((pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)==TASK_FLAGS_ENDTASK){
		if(push_back(&(gs_stScheduler.stWaitList), pstRunningTask)==FALSE){
			kUnLockForSystemData(bPreviousFlag);
			kPrintf("kAddTaskToReadyList() Failed... \n");
			return;
		}
		//���ý�Ʈ ����Ī �ٽ� �κ�
		/* �򰥸��� �κ�
		 * ���� ���ο� �½�ũ�� ���ý�Ʈ�� �ٲ�ٸ�, RFLAGS ���� �ٲ�Ƿ� �ٽ� ���ͷ�Ʈ�� Ȱ��ȭ �� �� �ִ�.
		 * ���� ���ο� �½�ũ�� �ƴ϶�, ����� �½�ũ��� ���� ����� RFLAGS�� �����ϹǷ� ���ͷ�Ʈ�� �ٽ� Ȱ��ȭ �� �� �ִ�.
		 * ���ο� �½�ũ�� ����� ���� ���ͷ�Ʈ Ȱ��ȭ ��Ʈ�� üũ�Ǿ��ִ��� Ȯ���� ����.
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
	 * ���ͷ�Ʈ���� �߻��Ǵ� �κ��̹Ƿ� ���ͷ�Ʈ�� ��߻��� ������ ���ص� �Ǵ� �ǰ�?
	 * ���ͷ�Ʈ���� context switching�� �Ϸ���, �ܼ��� Ŀ�� ���ÿ� ����� CONTEXT�� ���� �½�ũ�� CONTEXT�� �ٲٸ� �ȴ�.
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
	//���� �½�ũ��� �ð� ����
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE)==TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
	}
	//���� ������� �½�ũ
	//���ý�Ʈ ������ �����ʰ� �ҷ����⸸ �Ѵ�.
	if((pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)==TASK_FLAGS_ENDTASK){
		push_back(&(gs_stScheduler.stWaitList), pstRunningTask);
	}
	//�Ϲ� �½�ũ ó��
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
	//���ý�Ʈ ����Ī �ٽ� �κ�
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
	QWORD qwLastMeasureTickCount ,qwLastSpendTickInIdleTask;
	QWORD qwCurrentMeasureTickCount ,qwCurrentSpendTickInIdleTask;
	QWORD qwTaskID;
	BOOL bPreviousFlag;
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
				kPrintf("IDLE: Task ID[0x%q] is completely ended.\n",\
						pstTask->stLinkedList.Node_ID);
				qwTaskID = pstTask->stLinkedList.Node_ID;
				kFreeTCB(qwTaskID);
				kUnLockForSystemData(bPreviousFlag);
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
