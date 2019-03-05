/*
 * InterruptHandler.c
 *
 *  Created on: 2019. 2. 13.
 *      Author: DEWH
 */
#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "ConsoleShell.h"

void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode){
	//QWORD* i=0x800000UL;
	//int j;
	//for(j=0;j<24;j++){
		//kPrintf("%q: %q\t",i-j,*(i-j));
	//}
	char vcBuffer[3]={0,};
	vcBuffer[0]='0' + iVectorNumber / 10;
	vcBuffer[1]='0' + iVectorNumber % 10;
	kPrintStringXY(0, 0, "=========================================================");
	kPrintStringXY(0, 1, "                   Exception Occured...               ");
	kPrintStringXY(0, 2, "                       vector:                          ");
	kPrintStringXY(30, 2, vcBuffer);
	kPrintStringXY(0, 3, "=========================================================");
	//kReadContext((char*)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT));
	while(1);
}
void kCommonInterruptHandler(int iVectorNumber){
	char vcBuffer[]="[INT:  , ]";
	static int g_iCommonInterruptCount= 0;
	vcBuffer[5]='0' + iVectorNumber / 10;
	vcBuffer[6]='0' + iVectorNumber % 10;

	vcBuffer[8]='0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1 ) % 10;
	kPrintStringXY(70, 0, vcBuffer);
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}
void kKeyboardHandler(int iVectorNumber){
	char vcBuffer[]="[INT:  , ]";
	static int g_iCommonInterruptCount= 0;
	BYTE bTemp;
	vcBuffer[5]='0' + iVectorNumber / 10;
	vcBuffer[6]='0' + iVectorNumber % 10;

	vcBuffer[8]='0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1 ) % 10;
	kPrintStringXY(0, 0, vcBuffer);

	if(kIsOutputBufferFull()==TRUE)
	{
		bTemp = kGetKeyboardScanCode();
		kConvertScanCodeAndPutQueue(bTemp);
	}

	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

}
void kTimerHandler(int iVectorNumber){
	char vcBuffer[]="[INT:  , ]";
	static int g_iCommonInterruptCount= 0;
	BYTE bTemp;
	vcBuffer[5]='0' + iVectorNumber / 10;
	vcBuffer[6]='0' + iVectorNumber % 10;

	vcBuffer[8]='0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1 ) % 10;
	kPrintStringXY(70, 1, vcBuffer);


	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

	g_qwTickCount++;
	kDecreaseProcessorTime();
	if(kIsProcessorTimeExpired()==TRUE){
		kScheduleInterrupt();
	}
}
void kDeviceNotAvailableHandler(int iVectorNumber)
{
	/*
	 * FPU 연산을 하려던 와중 TS 비트가 설정된 경우 실행되는 핸들러이다.
	 *
	 */
	char vcBuffer[]="[EXC:  , ]";
	static int g_iCommonInterruptCount= 0;
	BYTE bTemp;
	FPUCONTEXT* pstFPUContext;
	TCB* pstCurrentTCB;
	QWORD qwLastFPUUsedTaskID;

	vcBuffer[5]='0' + iVectorNumber / 10;
	vcBuffer[6]='0' + iVectorNumber % 10;

	vcBuffer[8]='0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1 ) % 10;
	kPrintStringXY(0, 0, vcBuffer);

	kClearTS();
	pstFPUContext = (FPUCONTEXT*)TASK_FPUCONTEXTPOOLADDRESS;

	qwLastFPUUsedTaskID = kGetLastFPUUSEDTaskID();
	pstCurrentTCB = kGetRunningTCB();

	if(qwLastFPUUsedTaskID==kGetRunningTCB()->stLinkedList.Node_ID){
		return;
	}else if(qwLastFPUUsedTaskID != TASK_INVALIDID){
		kSaveFPUContext(&(pstFPUContext[GETTCBOFFSET(qwLastFPUUsedTaskID)]));
	}

	if(pstCurrentTCB->bFPUUsed == TRUE){
		kLoadFPUContext( &(pstFPUContext[GETTCBOFFSET(pstCurrentTCB->stLinkedList.Node_ID)]));
	}else{
		kInitializeFPU();
		pstCurrentTCB->bFPUUsed = TRUE;
		pstCurrentTCB->pstFPUContext = &(pstFPUContext[GETTCBOFFSET(pstCurrentTCB->stLinkedList.Node_ID)]);
	}
	kSetFPUUSEDTaskID(pstCurrentTCB->stLinkedList.Node_ID);
}

