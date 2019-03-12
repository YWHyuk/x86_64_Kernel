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
//태스크 풀관련 매크로
#define TASK_TCBPOOLADDRESS			0x800000UL
#define TASK_MAXCOUNT				1024

#define TASK_FPUCONTEXTPOOLADDRESS	((((TASK_TCBPOOLADDRESS + sizeof(TCB)*TASK_MAXCOUNT) >> 4) + 1) << 4)
#define TASK_STACKPOOLADDRESS		(TASK_FPUCONTEXTPOOLADDRESS + sizeof(FPUCONTEXT)*TASK_MAXCOUNT)
#define TASK_STACKSIZE				8192

#define TASK_INVALIDID				0xFFFFFFFFFFFFFFFF
//태스크가 최대 사용할 수 있는 시간
#define TASK_PROCESSORTIME			5
//태스크 우선순위 관련 매크로
#define TASK_MAXREADYLISTCOUNT		5
#define TASK_FLAGS_HIGHEST			0
#define TASK_FLAGS_HIGH				1
#define TASK_FLAGS_MEDIUM			2
#define TASK_FLAGS_LOW				3
#define TASK_FLAGS_LOWEST			4
#define TASK_FLAGS_WAIT				0xFF
//태스크의 플래그
#define TASK_FLAGS_ENDTASK			0x8000000000000000
#define TASK_FLAGS_SYSTEM			0X4000000000000000
#define TASK_FLAGS_PROCESS			0X2000000000000000
#define TASK_FLAGS_THREAD			0X1000000000000000
#define TASK_FLAGS_IDLE				0x0800000000000000
//플래그 필드에서 우선순위를 추출하고 변경하는 매크로
#define GETPRIORITY(x)				((x) & 0xFF)
#define SETPRIORITY(x, priority)	((x) = ((x) & 0xFFFFFFFFFFFFFF00 ) | (priority))
#define GETTCBOFFSET(x)				((x)&0xFFFFFFFF)
//TCB에서 멤버의 오프셋을 구하는 매크로 함수
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
	 *  ┌─────────────────────────────────────────────────────
	 *  | *TASK LINKED LIST-(TASK NODE ID,NEXT NODE...ETC)	  |
	 *  | 	TASK FLAG(TYPE OF TASK)							  |
	 *  | 	MEMORY INFORMATION(페이징 관련?)					  |
	 *  | 	<THREAD AREA>									  |
	 *  |┌────────────────────────────────────────────────── ||
	 *  ||*THREAD ID										 ||
	 *  ||*THREAD LIST										 ||
	 *  ||	PARENT ID;										 ||
	 *  ||	CONTEXT											 ||
	 *	||	STACK INFORMATION								 ||
	 *  |└───────────────────────────────────────────────────||
	 *  └─────────────────────────────────────────────────────|
	 *
	 */
	//TASK AREA
	LINKEDLIST stLinkedList; // 이 구조체 내에 리스트가 담기는 구나
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
	/* 앞으로 구현해야 할 부분 */

	/* current 디렉토리, 파일 */
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
	//현재 실행 중인 태스크의 TCB
	TCB* pstRunningTCB;
	//실행 대기중인 태스크 링크드 리스트
	LINKEDLISTMANAGER stReadyList[TASK_MAXREADYLISTCOUNT];
	//종료할 태스크 링크드 리스트
	LINKEDLISTMANAGER stWaitList;
	//각 리스트마다 실행된 횟수
	int viExecuteCount[TASK_MAXREADYLISTCOUNT];
	//태스크에 허락된 CPU 시간
	int iProcessorTime;
	//프로세스 부하
	QWORD qwProcessorLoad;
	//유휴 태스크에서 소비한 시간
	QWORD qwSpendProcessorTimeInIdleTask;
	//QWORD
	QWORD qwLastFPUUsedTaskID;
}SCHEDULER;
#pragma pack( pop )
//================================================================
//태스크 관련 함수-초기화
//================================================================
void kInitializeTCBPool( void );
//================================================================
//태스크 관련 함수-할당 해제
//================================================================
static TCB* kAllocTCB( void );
static void kFreeTCB( QWORD qwID );
//================================================================
//태스크 관련 함수-생성
//================================================================
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress,QWORD qwMemorySize, QWORD qwEntryPointAddress);
static void kSetUpTask(TCB* pstTCB, QWORD qwFlags,QWORD qwEntryPointAddress ,\
		void* pvStackAddress, QWORD qwStackSize);
//================================================================
//태스크 관련 함수-우선순위 변경
//================================================================
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority);
//================================================================
//태스크 관련 함수-태스크 종료
//================================================================
BOOL kEndTask(QWORD qwTaskID);
void kExitTask( void );
//================================================================
//태스크 관련 함수-nodeId
//================================================================
TCB* kGetTCBInTCBPool(int iOffset);
//================================================================
//쓰레드 관련 함수
//================================================================
TCB* kGetProcessByThread(TCB* pstThread);
//================================================================
//스케줄러 관련 함수-초기화
//================================================================
void kInitializeScheduler( void );
//================================================================
//스케줄러 관련 함수-TCB* pstCurrentTCB 접근 함수;
//================================================================
void kSetRunningTCB( TCB* pstTask);
TCB* kGetRunningTCB( void );
//================================================================
//스케줄러 관련 함수-LIST* stReadyList 접근 함수;
//================================================================
static TCB* kGetNextTaskToRun( void );
static BOOL kAddTaskToReadyList( TCB* pstTask);
static TCB* kRemoveTaskReadyList( QWORD qwID );
int kGetReadyTaskCount( void );
int kGetTaskCount( void );
//================================================================
//스케줄러 관련 함수-iProcessorTime 겁근 함수
//================================================================
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );
//================================================================
//스케줄러 함수
//================================================================
void kSchedule(void);
BOOL kScheduleInterrupt(void);

BOOL kIsTaskExist(QWORD qwID);
QWORD kGetProcessorLoad( void );
void kIdleTask( void );
void kHaltProcessorByLoad( void );
//================================================================
//FPU 관련
//================================================================
QWORD kGetLastFPUUSEDTaskID( void );
void kSetFPUUSEDTaskID( QWORD qwTaskID);
//================================================================
//for debug
//================================================================
void kReadContext(CONTEXT* pstContext);
#endif /* __02_KERNEL64_SOURCE_TASK_H_ */
