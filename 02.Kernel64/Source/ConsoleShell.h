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
void kHelp( int iArgc, const char** pcArgv );
void kCls( int iArgc, const char** pcArgv );
void kShowTotalRamSize( int iArgc, const char** pcArgv );
void kStringToDecimalHexTest( int iArgc, const char** pcArgv );
void kShutdonw( int iArgc, const char** pcArgv );
void kSetTImer( int iArgc, const char** pcArgv );
void kWaitUsingPIT( int iArgc, const char** pcArgv );
void kReadTimeStampCounter( int iArgc, const char** pcArgv );
void kMeasureProcessorSpeed( int iArgc, const char** pcArgv );
void kShowDateTime( int iArgc, const char** pcArgv );
void kCreateTestTask( int iArgc, const char** pcArgv);
void kAutoComplete(char* vcCommandBuffer, int* iCommandBufferIndex);
#endif /* __02_KERNEL64_SOURCE_CONSOLESHELL_H_ */
