/*
 * Utility.h
 *
 *  Created on: 2019. 2. 12.
 *      Author: DEWH
 */
#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <stdarg.h>
#include "Types.h"
void kMemSet(void* pvDestination, BYTE bData, int iSize);
int kMemCpy(void* pvDestination, const void* pvSource, int iSize);
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize);
int kStrlen(const char* pcBuffer);
BOOL kSetInterruptFlag(BOOL bEnableInterrupt);
void kCheckTotalRAMSize( void );
QWORD kGetTotalRAMSize( void );
void kReverseString( char* pcBuffer);
long kAToI( const char* pcBuffer, int iRadix, long* lReturn);// 문자열을  인자로 준 진수형태로 해석해서 정수형으로 만든다.
QWORD kHexStringToQword( const char* pcBuffer);//
long kDecimalStringToLong( const char* pcBuffer);
int kIToA( long lValue, char* pcBuffer, int iRadix);
int kHexToString(QWORD qwValue, char* pcBuffer);
int kDecimalToString(long lvalue, char* pcBuffer);
int kSPrintf( char* pcBuffer, const char* pcFormatString, ...	);
int kVSPrintf( char* pcBuffer, const char* pcFormatString, va_list ap);
BOOL kIsPrintable(char cKey);
QWORD kGetTickCount(void);
extern volatile QWORD g_qwTickCount;
void kSleep(QWORD qwMillisecond);
#endif /* __UTILITY_H__ */
