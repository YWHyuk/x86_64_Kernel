/*
 * Synchronization.c
 *
 *  Created on: 2019. 2. 26.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_SYNCHRONIZATION_C_
#define __02_KERNEL64_SOURCE_SYNCHRONIZATION_C_
#include "Types.h"
#pragma pack(push, 1)
typedef struct kMutexStruct{
	volatile QWORD qwTaskID;
	volatile DWORD dwLockCount;

	volatile BOOL bLockFlag;

	BYTE vbPadding[3];
}MUTEX;
#pragma pack(pop)
//���ؽ� ���� �Լ�
void kInitializeMutex(MUTEX* pstMutex);
void kLock(MUTEX* pstMutex);
void kUnlock(MUTEX* pstMutex);
// ���ͷ�Ʈ �÷��� ���� �Լ�
BOOL kLockForSystemData( void );
void kUnLockForSystemData ( BOOL bInterruptFlag );

#endif /* __02_KERNEL64_SOURCE_SYNCHRONIZATION_C_ */
