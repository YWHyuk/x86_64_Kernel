/*
 * ISR.h
 *
 *  Created on: 2019. 2. 13.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_ISR_H_
#define __02_KERNEL64_SOURCE_ISR_H_
////////////////////////////////////////////////////////////////////////////////
//
//  context save, load function
//
////////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline))
static __inline__ void kSAVECONTEXT (void);
__attribute__((always_inline))
static __inline__ void kLOADCONTEXT (void);
////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
// 예외(Exception) 처리용 ISR
#include "Types.h"
void kISRDivideError( void );
void kISRDebug( void );
void kISRNMI( void );
void kISRBreakPoint( void );
void kISROverflow( void );
void kISRBoundRangeExceeded( void );
void kISRInvalidOpcode();
void kISRDeviceNotAvailable( void );
void kISRDoubleFault(void);//** );
void kISRCoprocessorSegmentOverrun( void );
void kISRInvalidTSS( void);//** );
void kISRSegmentNotPresent( void);//** );
void kISRStackSegmentFault( void);//** );
void kISRGeneralProtection( void);//** );
void kISRPageFault( void);//** );
void kISR15( void );
void kISRFPUError( void );
void kISRAlignmentCheck( void);//** );
void kISRMachineCheck( void );
void kISRSIMDError( void );
void kISRETCException( void );

// 인터럽트(Interrupt) 처리용 ISR
void kISRTimer( void );
void kISRKeyboard( void );
void kISRSlavePIC( void );
void kISRSerial2( void );
void kISRSerial1( void );
void kISRParallel2( void );
void kISRFloppy( void );
void kISRParallel1( void );
void kISRRTC( void );
void kISRReserved( void );
void kISRNotUsed1( void );
void kISRNotUsed2( void );
void kISRMouse( void );
void kISRCoprocessor( void );
void kISRHDD1( void );
void kISRHDD2( void );
void kISRETCInterrupt( void );
//For debugging...
void kReadMemory(int x,int y,QWORD* address);
void HexToChar(BYTE target,char* Dest);

#endif /* __02_KERNEL64_SOURCE_ISR_H_ */
