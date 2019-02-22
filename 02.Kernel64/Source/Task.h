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

#define TASK_STACKPOOLADDRESS		(TASK_TCBPOOLADDRESS + sizeof(TCB)*TASK_MAXCOUNT)
#define TASK_STACKSIZE				8192

#define TASK_INVALIDID				0xFFFFFFFFFFFFFFFF
//태스크가 최대 사용할 수 있는 시간
#define TASK_PROCESSORTIME			5

#pragma pack( push, 1 )
typedef struct kContextStruct{
	QWORD vqRegister[TASK_REGISTERCOUNT];
}CONTEXT;

typedef struct kTaskControlBlockStruct{
	LINKEDLIST stLinkedList; // 이 구조체 내에 리스트가 담기는 구나
	CONTEXT stContext;

	QWORD qwFlags;

	void* pvStackAddress;
	QWORD qwStackSize;

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
	LINKEDLISTMANAGER stReadyList;
	//태스크에 허락된 CPU 시간
	int iProcessorTime;
}SCHEDULER;
#pragma pack( pop )
//태스크 관련 함수-초기화
void kInitializeTCBPool( void );
//태스크 관련 함수-할당 해제
TCB* kAllocTCB( void );
void kFreeTCB( QWORD qwID );
//태스크 관련 함수-생성
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress);
void kSetUpTask(TCB* pstTCB, QWORD qwFlags,QWORD qwEntryPointAddress ,\
		void* pvStackAddress, QWORD qwStackSize);
//스케줄러 관련 함수-초기화
void kInitializeScheduler( void );
//스케줄러 관련 함수-TCB* pstCurrentTCB 접근 함수;
void kSetRunningTCB( TCB* pstTask);
TCB* kGetRunningTCB( void );
//스케줄러 관련 함수-LIST stReadyList 접근 함수;
TCB* kGetNextTaskToRun( void );
BOOL kAddTaskToReadyList( TCB* pstTask);
//스케줄러 관련 함수-iProcessorTime 겁근 함수
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );
//스케줄러 함수
void kSchedule(void);
BOOL kScheduleInterrupt(void);
//for debug
void kReadContext(CONTEXT* pstContext);
#endif /* __02_KERNEL64_SOURCE_TASK_H_ */
