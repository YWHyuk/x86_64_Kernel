/*
 * InterruptHandler.h
 *
 *  Created on: 2019. 2. 13.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_INTERRUPTHANDLER_H_
#define __02_KERNEL64_SOURCE_INTERRUPTHANDLER_H_
#include "Types.h"


void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void kCommonInterruptHandler(int iVectorNumber);
void kKeyboardHandler(int iVectorNumber);
void kTimerHandler(int iVectorNumber);
void kHDDHandler(int iVectorNumber);
void kDeviceNotAvailableHandler(int iVectorNumber);
#endif /* __02_KERNEL64_SOURCE_INTERRUPTHANDLER_H_ */
