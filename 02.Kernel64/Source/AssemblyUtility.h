#ifndef __ASSEMBLYUTILITY_H__
#define __ASSEMBLYUTILITY_H__
#include "Types.h"
#include "Task.h"
BYTE kInPortByte(WORD wPort);
void kOutPortByte(WORD wPort,WORD wData);
WORD kInPortWord(WORD wPort);
void kOutPortWord(WORD wPort,WORD wData);
void kLoadGDTR(QWORD qwGDTRAddress);
void kLoadTR(WORD wTSSSegmentOffset);
void kLoadIDTR(QWORD qwIDTRAddress);
void kEnableInterrupt(void);
void kDisableInterrupt(void);
QWORD kReadRFLAGS(void);
//void kStackSearch(void);
void kSoftInterrupt(void);
QWORD kReadTSC(void);
void kContextSwitch(CONTEXT* pstCurrentContext, CONTEXT* pstNextContext);
void kHlt( void );
void kInitializeFPU( void );
void kSaveFPUContext( void* pvFPUContext );
void kLoadFPUContext( void* pvFPUContext );
void kSetTS( void );
void kClearTS( void );
//pbDestination의 값이 bCompare과  같다면 bSource의 값으로 바꾸고 return TRUE
//아니라면 return FALSE
BOOL kTestAndSet( volatile BYTE* pbDestination, BYTE bCompare, BYTE bSource);
void kEnableGlobalLocalAPIC( void );
#endif
