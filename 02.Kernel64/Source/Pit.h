/*
 * Pit.h
 *
 *  Created on: 2019. 2. 18.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_PIT_H_
#define __02_KERNEL64_SOURCE_PIT_H_
#include "Types.h"
#include "AssemblyUtility.h"
#define PIT_FREQUENCY			1193182
#define MSTOCOUNT( x )			(PIT_FREQUENCY * ( x ) / 1000)
#define USTOCOUNT( x )			(PIT_FREQUENCY * ( x ) / 1000000)
//PIT IO Map Port address
#define PIT_COUNTER0_PORT		0x40
#define PIT_CONTROL_PORT		0x43
/*7~4bit field*/
#define PIT_CONTROL_SC_COUNTER0			0x00
#define PIT_CONTROL_SC_COUNTER1			0x40
#define PIT_CONTROL_SC_COUNTER2			0x80
#define PIT_CONTROL_RW_2BYTE_READ		0x00/* You have to set 0 on 3~0 bit */
#define	PIT_CONTROL_RW_LOWER_BYTE_RW	0x10
#define PIT_CONTROL_RW_UPPER_BYTE_RW	0x20
#define PIT_CONTROL_RW_2BYTE_RDWR		0x30
/*3~1bit filed*/
#define PIT_CONTROL_MODE0				0x00//Interrupt during counting
#define PIT_CONTROL_MODE1				0x02
#define PIT_CONTROL_MODE2				0x04//Clock rate generator
#define PIT_CONTROL_MODE3				0x06
#define PIT_CONTROL_MODE4				0x08
#define PIT_CONTROL_MODE5				0x0A
/*0bit filed*/
#define PIT_CONTROL_BCD					0x01
#define PIT_CONTROL_BINARY				0x00

#define PIT_COUNTER0_ONCE				PIT_CONTROL_SC_COUNTER0 | PIT_CONTROL_RW_2BYTE_RDWR |\
										PIT_CONTROL_MODE0 | PIT_CONTROL_BINARY
#define PIT_COUNTER0_PERIODIC			PIT_CONTROL_SC_COUNTER0 | PIT_CONTROL_RW_2BYTE_RDWR |\
										PIT_CONTROL_MODE2 | PIT_CONTROL_BINARY

void kInitializePIT(WORD wCount, BOOL bPeriodic);
WORD kReadCounter0( void );
void kWaitUsingDirectPIT(WORD wCount);
void kWaitms(long lMillisecond);
#endif /* 02_KERNEL64_SOURCE_PIT_H_ */
