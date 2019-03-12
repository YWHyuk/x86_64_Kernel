/*
 * Task.h
 *
 *  Created on: 2019. 2. 19.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_TASK_H_
#define __02_KERNEL64_SOURCE_TASK_H_
#include "Types.h"
#include "LinkedList.h"
#define TASK_REGISTERCOUNT			(5 + 19)
#define TASK_REGISTERSIZE			8

#define TASK_GSOFFSET				0
#define TASK_FSOFFSET				1
#define TASK_ESOFFSET				2
#define TASK_DSOFFSET				3
#define TASK_R15OFFSET				4
#define TASK_R14OFFSET				5
#define TASK_R13OFFSET				6
#define TASK_R12OFFSET				7
#define TASK_R11OFFSET				8
#define TASK_R10OFFSET				9
#define TASK_R9OFFSET				10
#define TASK_R8OFFSET				11
#define TASK_RSIOFFSET				12
#define TASK_RDIOFFSET				13
#define TASK_RDXOFFSET				14
#define TASK_RCXOFFSET				15
#define TASK_RBXOFFSET				16
#define TASK_RAXOFFSET				17
#define TASK_RBPOFFSET				18
#define TASK_RIPOFFSET				19
#define TASK_CSOFFSET				20
#define TASK_RFLAGSOFFSET			21
#define TASK_RSPOFFSET				22
#define TASK_SSOFFSET				23
//�½�ũ Ǯ���� ��ũ��
#define TASK_TCBPOOLADDRESS			0x800000UL
#define TASK_MAXCOUNT				1024

#define TASK_FPUCONTEXTPOOLADDRESS	((((TASK_TCBPOOLADDRESS + sizeof(TCB)*TASK_MAXCOUNT) >> 4) + 1) << 4)
#define TASK_STACKPOOLADDRESS		(TASK_FPUCONTEXTPOOLADDRESS + sizeof(FPUCONTEXT)*TASK_MAXCOUNT)
#define TASK_STACKSIZE				8192

#define TASK_INVALIDID				0xFFFFFFFFFFFFFFFF
//�½�ũ�� �ִ� ����� �� �ִ� �ð�
#define TASK_PROCESSORTIME			5
//�½�ũ �켱���� ���� ��ũ��
#define TASK_MAXREADYLISTCOUNT		5
#define TASK_FLAGS_HIGHEST			0
#define TASK_FLAGS_HIGH				1
#define TASK_FLAGS_MEDIUM			2
#define TASK_FLAGS_LOW				3
#define TASK_FLAGS_LOWEST			4
#define TASK_FLAGS_WAIT				0xFF
//�½�ũ�� �÷���
#define TASK_FLAGS_ENDTASK			0x8000000000000000
#define TASK_FLAGS_SYSTEM			0X4000000000000000
#define TASK_FLAGS_PROCESS			0X2000000000000000
#define TASK_FLAGS_THREAD			0X1000000000000000
#define TASK_FLAGS_IDLE				0x0800000000000000
//�÷��� �ʵ忡�� �켱������ �����ϰ� �����ϴ� ��ũ��
#define GETPRIORITY(x)				((x) & 0xFF)
#define SETPRIORITY(x, priority)	((x) = ((x) & 0xFFFFFFFFFFFFFF00 ) | (priority))
#define GETTCBOFFSET(x)				((x)&0xFFFFFFFF)
//TCB���� ����� �������� ���ϴ� ��ũ�� �Լ�
#define GETTCBFROMTHREADLINK( x )		( TCB* )( ( QWORD )( x ) - offsetof( TCB , stThreadLinkedList ))
#pragma pack( push, 1 )
typedef struct kContextStruct{
	QWORD vqRegister[TASK_REGISTERCOUNT];
}CONTEXT;
typedef struct kFPUContext{
	QWORD vqwFPUContext[512/8];
}FPUCONTEXT;
typedef struct kTaskControlBlockStruct{
	/*
	 *     <TASK CONTROL BLOCK>
	 *  ������������������������������������������������������������������������������������������������������������
	 *  | *TASK LINKED LIST-(TASK NODE ID,NEXT NODE...ETC)	  |
	 *  | 	TASK FLAG(TYPE OF TASK)							  |
	 *  | 	MEMORY INFORMATION(����¡ ����?)					  |
	 *  | 	<THREAD AREA>									  |
	 *  |������������������������������������������������������������������������������������������������������ ||
	 *  ||*THREAD ID										 ||
	 *  ||*THREAD LIST										 ||
	 *  ||	PARENT ID;										 ||
	 *  ||	CONTEXT											 ||
	 *	||	STACK INFORMATION								 ||
	 *  |��������������������������������������������������������������������������������������������������������||
	 *  ������������������������������������������������������������������������������������������������������������|
	 *
	 */
	//TASK AREA
	LINKEDLIST stLinkedList; // �� ����ü ���� ����Ʈ�� ���� ����
	QWORD qwFlags;
	void* pvMemoryAddress;
	QWORD qwMemorySize;

	//THREAD AREA
	LINKEDLIST stThreadLinkedList;
	LINKEDLISTMANAGER stChildThreadList;

	QWORD qwParentTaskID;
	CONTEXT stContext;
	void* pvStackAddress;
	QWORD qwStackSize;
	//FPU AREA
	FPUCONTEXT* pstFPUContext;
	BOOL bFPUUsed;
	/* ������ �����ؾ� �� �κ� */

	/* current ���丮, ���� */
	/* waiting for descriptor */
	/* descriptor table */
	/* uid gid */
	/* page table */
}TCB;


typedef struct kTCBPoolManagerStruct{
	TCB* pstTCBStartAddress;
	int iTaskCount;
	int iMaxCount;
	DWORD dwAllocatedCount;
}TCBPOOLMANAGER;

typedef struct kSchedulerStruct{
	//���� ���� ���� �½�ũ�� TCB
	TCB* pstRunningTCB;
	//���� ������� �½�ũ ��ũ�� ����Ʈ
	LINKEDLISTMANAGER stReadyList[TASK_MAXREADYLISTCOUNT];
	//������ �½�ũ ��ũ�� ����Ʈ
	LINKEDLISTMANAGER stWaitList;
	//�� ����Ʈ���� ����� Ƚ��
	int viExecuteCount[TASK_MAXREADYLISTCOUNT];
	//�½�ũ�� ����� CPU �ð�
	int iProcessorTime;
	//���μ��� ����
	QWORD qwProcessorLoad;
	//���� �½�ũ���� �Һ��� �ð�
	QWORD qwSpendProcessorTimeInIdleTask;
	//QWORD
	QWORD qwLastFPUUsedTaskID;
}SCHEDULER;
#pragma pack( pop )
//================================================================
//�½�ũ ���� �Լ�-�ʱ�ȭ
//================================================================
void kInitializeTCBPool( void );
//================================================================
//�½�ũ ���� �Լ�-�Ҵ� ����
//================================================================
static TCB* kAllocTCB( void );
static void kFreeTCB( QWORD qwID );
//================================================================
//�½�ũ ���� �Լ�-����
//================================================================
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress,QWORD qwMemorySize, QWORD qwEntryPointAddress);
static void kSetUpTask(TCB* pstTCB, QWORD qwFlags,QWORD qwEntryPointAddress ,\
		void* pvStackAddress, QWORD qwStackSize);
//================================================================
//�½�ũ ���� �Լ�-�켱���� ����
//================================================================
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority);
//================================================================
//�½�ũ ���� �Լ�-�½�ũ ����
//================================================================
BOOL kEndTask(QWORD qwTaskID);
void kExitTask( void );
//================================================================
//�½�ũ ���� �Լ�-nodeId
//================================================================
TCB* kGetTCBInTCBPool(int iOffset);
//================================================================
//������ ���� �Լ�
//================================================================
TCB* kGetProcessByThread(TCB* pstThread);
//================================================================
//�����ٷ� ���� �Լ�-�ʱ�ȭ
//================================================================
void kInitializeScheduler( void );
//================================================================
//�����ٷ� ���� �Լ�-TCB* pstCurrentTCB ���� �Լ�;
//================================================================
void kSetRunningTCB( TCB* pstTask);
TCB* kGetRunningTCB( void );
//================================================================
//�����ٷ� ���� �Լ�-LIST* stReadyList ���� �Լ�;
//================================================================
static TCB* kGetNextTaskToRun( void );
static BOOL kAddTaskToReadyList( TCB* pstTask);
static TCB* kRemoveTaskReadyList( QWORD qwID );
int kGetReadyTaskCount( void );
int kGetTaskCount( void );
//================================================================
//�����ٷ� ���� �Լ�-iProcessorTime �̱� �Լ�
//================================================================
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );
//================================================================
//�����ٷ� �Լ�
//================================================================
void kSchedule(void);
BOOL kScheduleInterrupt(void);

BOOL kIsTaskExist(QWORD qwID);
QWORD kGetProcessorLoad( void );
void kIdleTask( void );
void kHaltProcessorByLoad( void );
//================================================================
//FPU ����
//================================================================
QWORD kGetLastFPUUSEDTaskID( void );
void kSetFPUUSEDTaskID( QWORD qwTaskID);
//================================================================
//for debug
//================================================================
void kReadContext(CONTEXT* pstContext);
#endif /* __02_KERNEL64_SOURCE_TASK_H_ */
