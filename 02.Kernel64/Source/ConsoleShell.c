/*
 * ConsoleShell.c
 *
 *  Created on: 2019. 2. 17.
 *      Author: DEWH
 */
#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Pit.h"
#include "RTC.h"
#include "AssemblyUtility.h"
SHELLCOMMANDENTRY gs_vstCommandTable[]={
		{"help", "Show Help",kHelp},
		{"cls", "Clear the Screen", kCls},
		{"totalram", "Show a Total Ram Size", kShowTotalRamSize},
		{"strtod", "String To Radix Convert",kStringToDecimalHexTest},
		{"shutdown", "Power off the machine",kShutdonw},
		{"strtod", "String To Decial/Hex Convert",kStringToDecimalHexTest},
		{"settimer","Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",kSetTImer},
		{"wait","Wait ms Using PIT, ex)wait 100(ms)",kWaitUsingPIT},
		{"rdtsc","Read Time Stamp Counter", kReadTimeStampCounter},
		{"cpuspeed","Measure Processor Speed", kMeasureProcessorSpeed},
		{"date","Show Date And Time",kShowDateTime},
		{"createtask","Create Task",kCreateTestTask}
};
void kStartConsoleShell(void){
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int iCommandBufferIndex=0;
	BYTE bKey;
	int iCursorX, iCursorY;
	kPrintf("MINT64>");
	while(1){
		bKey = kGetCh();
		switch(bKey){
		case KEY_BACKSPACE:
			if(iCommandBufferIndex>0){//현재 버퍼 입력에 아무 것도 없다면 작동 안함.
				//프롬프트 처리
				kGetCursor(&iCursorX, &iCursorY);
				kPrintStringXY(iCursorX -1 ,iCursorY, " ");
				kSetCursor(iCursorX-1, iCursorY );
				//command 버퍼 처리
				vcCommandBuffer[--iCommandBufferIndex]='\0';
			}
			break;
		case KEY_ENTER:
			//프롬프트 처리 1
			kPrintf("\n");

			//커맨드 버퍼 처리
			vcCommandBuffer[iCommandBufferIndex]='\0';
			kExecuteCommand(vcCommandBuffer);
			kMemSet(vcCommandBuffer, 0, CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			iCommandBufferIndex=0;

			//프롬프트 처리 2
			kPrintf("%s","MINT64>");

			break;
		case KEY_LSHIFT:
		case KEY_RSHIFT:
		case KEY_CAPSLOCK:
		case KEY_NUMLOCK:
		case KEY_SCROLLLOCK:
			break;
		default:
			//프롬프트 처리
			kPrintf("%c",bKey);
			//커맨드 버퍼 처리
			if(iCommandBufferIndex==CONSOLESHELL_MAXCOMMANDBUFFERCOUNT){
				kPrintf("Shell buffer is full...\nYou have Press Enter...\n");
				break;
			}
			vcCommandBuffer[iCommandBufferIndex++]=bKey;
		}
	}
}
void kExecuteCommand(const char* pcCommandBuffer){
	/*
	 *pcComandBuffer에서 실행시키는 명령어를 얻어야 한다. 하지만
	 *실행시키려면 정확한 명령어를 얻어내야한다.
	 *즉, 공백문자는 처리하고 비교해야 한다.
	 *또한 명령과, 인자 옵션 등을 구분해야 한다.
	 *보편적인 형태로 argc, argv형태로 전달하는 방식으로 만들어 보자.
	 */
	int iArgc=0,iReturn;
	int iBufferIndex=0;
	int iArgvIndex=0;
	int iBufferLength,i;
	BOOL bStart=0;
	char* ppcArgv[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT]={0,};
	char pcArgv[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT]={0,};
	CommandFunction pfFunction=NULL;

	iBufferLength = kStrlen(pcCommandBuffer);

	while(iBufferIndex < iBufferLength){
		//처음에 있는 공백 문자 처리..
		while(kIsPrintable(pcCommandBuffer[iBufferIndex])==FALSE){
			iBufferIndex++;
		}
		bStart=FALSE;
		while(kIsPrintable(pcCommandBuffer[iBufferIndex])){
			if(bStart == FALSE){
				ppcArgv[iArgc]= pcArgv + iArgvIndex;
				iArgc++;
				bStart=TRUE;
			}
			pcArgv[iArgvIndex] = pcCommandBuffer[iBufferIndex];
			iBufferIndex++;
			iArgvIndex++;
		}
		if(iArgvIndex!=0){
			if(kIsPrintable(pcArgv[iArgvIndex-1])){
				pcArgv[iArgvIndex] = '\0';
				iArgvIndex++;
			}
		}
	}
	for(i=0;i<sizeof(gs_vstCommandTable)/sizeof(SHELLCOMMANDENTRY);i++){
		if(kStrlen(pcArgv) == kStrlen(gs_vstCommandTable[i].pcCommand)){
			if(!kMemCmp(pcArgv, gs_vstCommandTable[i].pcCommand, kStrlen(pcArgv))){
				pfFunction = gs_vstCommandTable[i].pfFunction;
				break;
			}
		}
	}
	if(pfFunction == NULL){
		kPrintf("'%s' is not found...\n",pcArgv);
		return;
	}
	pfFunction(iArgc,ppcArgv);
}
//void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter){}
//int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter){}
//실제 커맨드 함수
void kHelp( int iArgc, const char** pcArgv ){
	int iCount;
	int i,iCommandNumber;
	int iX,iY;
	int iMaxCommandLength=0;
	int iTemp;
	kPrintf("=======================================================\n");
	kPrintf("                    Mint64 Shell help                  \n");
	kPrintf("=======================================================\n");
	iCommandNumber = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for(i=0;i<iCommandNumber;i++){
		iTemp=kStrlen(gs_vstCommandTable[i]);
		if(iMaxCommandLength < iTemp)
			iMaxCommandLength = iTemp;
	}
	for(i=0;i<iCommandNumber;i++){
		kPrintf("  %s",gs_vstCommandTable[i].pcCommand);
		kGetCursor(&iX, &iY);
		kSetCursor(iMaxCommandLength + 12 , iY );
		kPrintf("-%s\n",gs_vstCommandTable[i].pcHelp);
	}
	kPrintf("\n");
}
void kCls( int iArgc, const char** pcArgv ){
	kClearScreen();
}
void kShowTotalRamSize( int iArgc, const char** pcArgv ){
	kPrintf("This machine has %dMB...\n",kGetTotalRAMSize());
}
void kStringToDecimalHexTest( int iArgc, const char** pcArgv ){
	char pcBuffer[100];
	long lValue;
	int i;
	for(i = 1; i < iArgc; i++){
		kPrintf("Param[%d] = '%s', ",i,pcArgv[i]);
		if(kMemCmp(pcArgv[i],"0x",2) == 0 ){
			lValue = kAToI((pcArgv[i])+2, 16);
			if(lValue==-1){
				kPrintf("Not Supported Format...\n");
			}
			else{
				kPrintf("Hex Value = %q\n", lValue);
			}
		}
		else{
			lValue = kAToI(pcArgv[i], 10);
			if(lValue==-1){
				kPrintf("Not Supported Format...\n");
			}
			else{
				kPrintf("Decimal Value = %d\n", lValue);
			}
		}
	}
}
void kShutdonw( int iArgc, const char** pcArgv ){
	kPrintf("System Shutdown Start...\n");
	kPrintf("Press Any Key to Reboot PC...");
	kGetCh();
	kReboot();
}
void kSetTImer( int iArgc, const char** pcArgv ){
	WORD wCount;
	BOOL bPeriodic;
	if(iArgc != 3){
		kPrintf("%s\n","Wrong Parmeter.. ex)settimer 10(ms) 1(periodic)");
		return;
	}
	wCount= kAToI(pcArgv[1], 10);
	bPeriodic = kAToI(pcArgv[2], 10);
	kPrintf("Time = %d ms, Periodic = %d Change Complete\n",wCount,bPeriodic);
	kInitializePIT(MSTOCOUNT(wCount), bPeriodic);
}
void kWaitUsingPIT( int iArgc, const char** pcArgv ){
	long lValue;
	int i;
	if(iArgc != 2){
		kPrintf("%s\n","Wrong Parmeter.. ex)wait 100(ms)");
		return;
	}
	lValue = kAToI(pcArgv[1], 10);
	kPrintf("%d ms Sleep Start...\n",lValue);
	kDisableInterrupt();
	for(i=0;i<lValue/30;i++){
		kWaitUsingDirectPIT(MSTOCOUNT(30));
		kPrintf(".");
	}
	kWaitUsingDirectPIT(MSTOCOUNT(lValue % 30));
	kEnableInterrupt();
	kPrintf("%d ms Sleep Complete\n",lValue);
	kInitializePIT(MSTOCOUNT(1), TRUE);
}
void kReadTimeStampCounter( int iArgc, const char** pcArgv ){
	QWORD qwTSC;
	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n",qwTSC);
}
void kMeasureProcessorSpeed( int iArgc, const char** pcArgv ){
	int i;
	QWORD qwLastTSC, qwTotalTSC =0;
	kPrintf("Now Measuring.");
	kDisableInterrupt();
	for(i = 0; i<200; i++){
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;
		kPrintf(".");
	}
	kInitializePIT(MSTOCOUNT(1), TRUE);
	kEnableInterrupt();
	kPrintf("\nCPU Speed = %d MHz\n",qwTotalTSC/10/1000/1000);
}
void kShowDateTime( int iArgc, const char** pcArgv ){
	BYTE bSecond,bMinute,bHour;
	BYTE bDayOfWeek,bDayOfMonth, bMonth;
	WORD wYear;

	kReadRTCTIME(&bHour, &bMinute, &bSecond);
	kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

	kPrintf("Date: %d %d %d %s, ", wYear, bMonth, bDayOfMonth,kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d %d %d\n",bHour,bMinute,bSecond);
}
void kTestTask1(void){
	BYTE bData;
	int i = 0, iX = 0, iY = 0, iMargin;
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEDMEMORYADDRESS;
	TCB* pstRunningTask;

	pstRunningTask = kGetRunningTCB();
	iMargin = (pstRunningTask->stLinkedList.Node_ID & 0xFFFFFFFFUL) % 10;
	kPrintf("kTestTask1:%d\n",iMargin);
	while(1){
		switch(i){
		case 0:
			iX++;
			if(iX>= (CONSOLE_WIDTH - iMargin))
				i=1;
			break;
		case 1:
			iY++;
			if(iY >= (CONSOLE_HEIGHT - iMargin))
				i=2;
			break;
		case 2:
			iX--;
			if(iX<iMargin)
				i=3;
			break;
		case 3:
			iY--;
			if(iY<iMargin)
				i=0;
			break;
		}
		pstScreen[iY* CONSOLE_WIDTH + iX].bCharactor = bData;
		pstScreen[iY* CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
		bData++;
		//kSchedule();
	}
}
void kTestTask2(void){
	int i=0,iOffset;
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEDMEMORYADDRESS;
	TCB* pstRunningTask;
	char vcData[4] = {'-','\\','|','/'};

	pstRunningTask = kGetRunningTCB();
	iOffset = (pstRunningTask->stLinkedList.Node_ID & 0xFFFFFFFFUL) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset %(CONSOLE_WIDTH * CONSOLE_HEIGHT));
	while(1){
		pstScreen[iOffset].bCharactor = vcData[i%4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
		i++;
		kSchedule();
	}
}
void kCreateTestTask( int iArgc, const char** pcArgv ){
	int i;
	if(iArgc != 3 ){
		kPrintf("%s\n","Wrong Parmeter..");
	}
	switch(kAToI(pcArgv[1], 10)){
	case 1:
		for(i=0;i<kAToI(pcArgv[2], 10);i++){
			if(kCreateTask(0, (QWORD)kTestTask1)==NULL)
				break;
		}
		kPrintf("Task1 %d Created\n",i);
		break;
	case 2:
		for(i=0;i<kAToI(pcArgv[2], 10);i++){
			if(kCreateTask(0, (QWORD)kTestTask2)==NULL)
				break;
		}
		kPrintf("Task2 %d Created\n",i);
		break;
	}
}
