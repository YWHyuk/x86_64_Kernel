/*
 * ISR.c
 *
 *  Created on: 2019. 2. 13.
 *      Author: DEWH
 */
#include "ISR.h"
__attribute__((always_inline))
static __inline__ void kSAVECONTEXT (void) {
	__asm__ __volatile__(
			//"push %rbp 			\n\t" *함수 호출시 기본적으로 생성됨
			//"movq %rsp, %rbp		\n\t" *함수 호출시 기본적으로 생성됨
			"push %rax 			\n\t"
			"push %rbx 			\n\t"
			"push %rcx 			\n\t"
			"push %rdx 			\n\t"
			"push %rdi 			\n\t"
			"push %rsi 			\n\t"
			"push %r8 			\n\t"
			"push %r9 			\n\t"
			"push %r10 			\n\t"
			"push %r11 			\n\t"
			"push %r12 			\n\t"
			"push %r13 			\n\t"
			"push %r14 			\n\t"
			"push %r15 			\n\t"
			"mov  %ds, %ax		\n\t"
			"push %rax 			\n\t"
			"mov  %es, %ax		\n\t"
			"push %rax 			\n\t"

			"mov  %fs, %ax		\n\t"
			"push %rax 			\n\t"
			"mov  %gs, %ax		\n\t"
			"push %rax 			\n\t"

		//	"push %fs 			\n\t"
		//	"push %gs 			\n\t"

			"mov  $0x10,%ax		\n\t"
			"mov  %ax, %ds		\n\t"
			"mov  %ax, %es		\n\t"
			"mov  %ax, %fs		\n\t"
			"mov  %ax, %gs		\n\t"
	);
}
__attribute__((always_inline))
static __inline__ void kLOADCONTEXT (void) {
	__asm__ __volatile__(
		//	"pop  %gs			\n\t"
		//	"pop  %fs        	\n\t"
			"pop  %rax			\n\t"
			"mov  %ax, %gs		\n\t"
			"pop  %rax			\n\t"
			"mov  %ax, %fs		\n\t"

			"pop  %rax			\n\t"
			"mov  %ax, %es		\n\t"
			"pop  %rax			\n\t"
			"mov  %ax, %ds		\n\t"
			"pop  %r15			\n\t"
			"pop  %r14			\n\t"
			"pop  %r13			\n\t"
			"pop  %r12			\n\t"
			"pop  %r11			\n\t"
			"pop  %r10			\n\t"
			"pop  %r9			\n\t"
			"pop  %r8			\n\t"
			"pop  %rsi 			\n\t"
			"pop  %rdi			\n\t"
			"pop  %rdx			\n\t"
			"pop  %rcx			\n\t"
			"pop  %rbx			\n\t"
			"pop  %rax			\n\t"
			"pop  %rbp			\n\t"
	);
}
#include "InterruptHandler.h"
void kISRDivideError(){
	kSAVECONTEXT();
	kCommonExceptionHandler(0, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRDebug(){
	kSAVECONTEXT();
	kCommonExceptionHandler(1, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRNMI( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(2, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRBreakPoint( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(3, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISROverflow( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(4, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRBoundRangeExceeded( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(5, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRInvalidOpcode(){
	kSAVECONTEXT();
	kCommonExceptionHandler(6, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRDeviceNotAvailable( void ){
	kSAVECONTEXT();
	kDeviceNotAvailableHandler(7);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRDoubleFault( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(8,0);// *(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8, %rsp \n\t"
			"iretq"
	);
}
void kISRCoprocessorSegmentOverrun( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(9, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRInvalidTSS( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(10,0);// *(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8, %rsp \n\t"
			"iretq"
	);
}
void kISRSegmentNotPresent( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(11,0);// *(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8,%rsp \n\t"
			"iretq"
	);
}
void kISRStackSegmentFault( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(12,0);// *(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8,%rsp \n\t"
			"iretq"
	);
}
void kISRGeneralProtection( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(13,0);// *(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8,%rsp \n\t"
			"iretq"
	);
}
void kISRPageFault( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(14, 0);//*(&qwErrCode - 0x01));
	kLOADCONTEXT();
	__asm__ __volatile__(
			"add $8,%rsp \n\t"
			"iretq"
	);
}
void kISR15( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(15, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRFPUError( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(16, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRAlignmentCheck( void ){//QWORD qwErrCode ){
	kSAVECONTEXT();
	kCommonExceptionHandler(17,0);// *(&qwErrCode - 0x01));
		kLOADCONTEXT();
		__asm__ __volatile__(
				"add $8,%rsp \n\t"
				"iretq"
		);
}
void kISRMachineCheck( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(18, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRSIMDError( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(19, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRETCException( void ){
	kSAVECONTEXT();
	kCommonExceptionHandler(20, 0);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}

// 인터럽트(Interrupt) 처리용 ISR
void kISRTimer( void ){
	kSAVECONTEXT();
	kTimerHandler(32);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRKeyboard( void ){
	kSAVECONTEXT();
	kKeyboardHandler(33);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRSlavePIC( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(34);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRSerial2( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(35);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRSerial1( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(36);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRParallel2( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(37);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRFloppy( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(38);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRParallel1( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(39);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRRTC( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(40);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRReserved( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(41);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRNotUsed1( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(42);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRNotUsed2( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(43);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRMouse( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(44);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRCoprocessor( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(45);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRHDD1( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(46);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRHDD2( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(47);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kISRETCInterrupt( void ){
	kSAVECONTEXT();
	kCommonInterruptHandler(48);
	kLOADCONTEXT();
	__asm__ __volatile__(
			"iretq"
	);
}
void kReadMemory(int x,int y,QWORD* address){
	BYTE* buffer=(BYTE*)address;
	int i=0;
	char string[20]={0,};
	char vcAddress[20]={0,};
	QWORD qwTemp=(QWORD)address;
	for(i=0;i<8;i++){
		HexToChar(qwTemp%256, vcAddress+14-2*i);
		qwTemp/=256;
	}
	for(i=0;i<8;i++){
		HexToChar(buffer[i], string+14-2*i);
	}
	kPrintStringXY(x , y, vcAddress);
	kPrintStringXY(x+ 17 , y, string);
}
void HexToChar(BYTE target,char* Dest){
	BYTE quote,remain;
	quote=target/16;
	remain=target%16;
	if(quote<10){
		Dest[0]=quote+'0';
	}else{
		Dest[0]=quote-10+'A';
	}
	if(remain<10){
			Dest[1]=remain+'0';
	}else{
			Dest[1]=remain-10+'A';
	}
}

