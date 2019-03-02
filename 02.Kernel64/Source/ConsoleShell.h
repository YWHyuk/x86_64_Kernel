/*
 * ConsoleShell.h
 *
 *  Created on: 2019. 2. 17.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_CONSOLESHELL_H_
#define __02_KERNEL64_SOURCE_CONSOLESHELL_H_
#include "Types.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT 	300
#define CONSOLESHELL_PROMPTMESSAGE 			"MINT64>"

typedef void (*CommandFunction)( int iArgc, const char** pcArgv );
#pragma pack( push, 1)
typedef struct kShellCommandEntryStruct{
	char* pcCommand;
	char* pcHelp;
	CommandFunction pfFunction;
}SHELLCOMMANDENTRY;
typedef struct kParameterListStruct{
	const char* pcBuffer;
	int iLength;
	int iCurrentPosition;
}PARAMETERLIST;
#pragma pack( pop )

void kStartConsoleShell(void);
void kExecuteCommand(const char* pcCommandBuffer);
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter);
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter);
//실제 커맨드 함수
static void kHelp( int iArgc, const char** pcArgv );
static void kCls( int iArgc, const char** pcArgv );
static void kShowTotalRamSize( int iArgc, const char** pcArgv );
static void kStringToDecimalHexTest( int iArgc, const char** pcArgv );
static void kShutdonw( int iArgc, const char** pcArgv );
static void kSetTImer( int iArgc, const char** pcArgv );
static void kWaitUsingPIT( int iArgc, const char** pcArgv );
static void kReadTimeStampCounter( int iArgc, const char** pcArgv );
static void kMeasureProcessorSpeed( int iArgc, const char** pcArgv );
static void kShowDateTime( int iArgc, const char** pcArgv );
static void kCreateTestTask( int iArgc, const char** pcArgv);
static void kChangeTaskPriority( int iArgc, const char** pcArgv);
static void kShowTaskList( int iArgc, const char** pcArgv);
static void kKillTask( int iArgc, const char** pcArgv);
static void kCPULoad( int iArgc, const char** pcArgv);
static void kCreateThreadTask(void);
static void kTestThread(int iArgc, const char** pcArgv);
static void kPrintNumberTask( int iArgc, const char** pcArgv );
static void kTestMutex( int iArgc, const char** pcArgv );
static void kShowMatrix( int iArgc, const char** pcArgv );
static void kFPUTestTask(void);
static void kTestPIE( int iArgc, const char** pcArgv );

void kAutoComplete(char* vcCommandBuffer, int* iCommandBufferIndex);
QWORD kRandom( void );
static void kDropCharactorThread(void);
static void kMatrixProcess(void);
#endif /* __02_KERNEL64_SOURCE_CONSOLESHELL_H_ */
