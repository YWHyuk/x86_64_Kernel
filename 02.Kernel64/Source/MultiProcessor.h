/*
 * MultiProcessor.h
 *
 *  Created on: 2019. 3. 19.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_MULTIPROCESSOR_H_
#define __02_KERNEL64_SOURCE_MULTIPROCESSOR_H_

#include "Types.h"

#define BOOTSTRAPPROCESSOR_FLAGADDRESS	0x7C09
#define MAXPROCESSORCOUNT				16

BOOL kStartUpApplicationProcessor( void );
BYTE kGetAPICID( void );
static BOOL kWakeUpApplicationProcessor( void );

#endif /* __02_KERNEL64_SOURCE_MULTIPROCESSOR_H_ */
