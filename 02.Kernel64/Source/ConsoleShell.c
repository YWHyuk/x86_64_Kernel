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
#include "Task.h"
#include "ISR.h"
#include "Synchronization.h"

SHELLCOMMANDENTRY gs_vstCommandTable[]={
		{"help", "Show Help",kHelp},
		{"cls", "Clear the Screen", kCls},
		{"totalram", "Show a Total Ram Size", kShowTotalRamSize},
		{"strtod", "String To Radix Convert",kStringToDecimalHexTest},
		{"shutdown", "Power off the machine",kShutdonw},
		{"settimer","Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",kSetTImer},
		{"wait","Wait ms Using PIT, ex)wait 100(ms)",kWaitUsingPIT},
		{"rdtsc","Read Time Stamp Counter", kReadTimeStampCounter},
		{"cpuspeed","Measure Processor Speed", kMeasureProcessorSpeed},
		{"date","Show Date And Time",kShowDateTime},
		{"createtask","Create Task",kCreateTestTask},
		{"changepriority","Change Task Priority, ex)changepriority 1(ID) 2(Priority)",\
				kChangeTaskPriority},
		{"tasklist","Show Task List",kShowTaskList},
		{"killtask","End task, ex)killtask 1(ID) or 0xffffffff(All task)",kKillTask},
		{"cpuload","Show Processor Load", kCPULoad},
		{"testmutex", "Test Mutex Function",kTestMutex}
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
		case KEY_TAB:
			kAutoComplete(vcCommandBuffer,&iCommandBufferIndex);
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
static void kHelp( int iArgc, const char** pcArgv ){
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
static void kCls( int iArgc, const char** pcArgv ){
	kClearScreen();
}
static void kShowTotalRamSize( int iArgc, const char** pcArgv ){
	kPrintf("This machine has %dMB...\n",kGetTotalRAMSize());
}
static void kStringToDecimalHexTest( int iArgc, const char** pcArgv ){
	long lValue;
	int i;
	for(i = 1; i < iArgc; i++){
		kPrintf("Param[%d] = '%s', ",i,pcArgv[i]);
		if(kMemCmp(pcArgv[i],"0x",2) == 0 ){
			kAToI(pcArgv[i]+2, 16, &lValue);
			if(lValue==-1){
				kPrintf("Not Supported Format...\n");
			}
			else{
				kPrintf("Hex Value = 0x%q\n", lValue);
			}
		}
		else{
			kAToI(pcArgv[i], 10,&lValue);
			if(lValue==-1){
				kPrintf("Not Supported Format...\n");
			}
			else{
				kPrintf("Decimal Value = %d\n", lValue);
			}
		}
	}
}
static void kShutdonw( int iArgc, const char** pcArgv ){
	kPrintf("System Shutdown Start...\n");
	kPrintf("Press Any Key to Reboot PC...");
	kGetCh();
	kReboot();
}
static void kSetTImer( int iArgc, const char** pcArgv ){
	QWORD qwCount;
	QWORD qwPeriodic;
	if(iArgc != 3){
		kPrintf("%s\n","Wrong Parmeter.. ex)settimer 10(ms) 1(periodic)");
		return;
	}
	kAToI(pcArgv[1], 10,&qwCount);
	kAToI(pcArgv[2], 10,&qwPeriodic);
	kPrintf("Time = %d ms, Periodic = %d Change Complete\n",qwCount,qwPeriodic);
	kInitializePIT(MSTOCOUNT(qwCount), qwPeriodic);
}
static void kWaitUsingPIT( int iArgc, const char** pcArgv ){
	long lValue;
	int i;
	if(iArgc != 2){
		kPrintf("%s\n","Wrong Parmeter.. ex)wait 100(ms)");
		return;
	}
	kAToI(pcArgv[1], 10,lValue);
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
static void kReadTimeStampCounter( int iArgc, const char** pcArgv ){
	QWORD qwTSC;
	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n",qwTSC);
}
static void kMeasureProcessorSpeed( int iArgc, const char** pcArgv ){
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
static void kShowDateTime( int iArgc, const char** pcArgv ){
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
static void kCreateTestTask( int iArgc, const char** pcArgv ){
	long i;
	long lValue;
	if(iArgc != 3 ){
		kPrintf("%s\n","Wrong Parmeter..");
	}
	switch(kAToI(pcArgv[1], 10,&lValue)){
	case 1:
		kAToI(pcArgv[2], 10,&lValue);
		for(i=0;i<lValue;i++){
			if(kCreateTask(TASK_FLAGS_LOW, (QWORD)kTestTask1)==NULL)
				break;
		}
		kPrintf("Task1 %d Created\n",i);
		break;
	case 2:
		kAToI(pcArgv[2], 10,&lValue);
		for(i=0;i<lValue;i++){
			if(kCreateTask(TASK_FLAGS_LOW, (QWORD)kTestTask2)==NULL)
				break;
		}
		kPrintf("Task2 %d Created\n",i);
		break;
	}
}
static void kChangeTaskPriority( int iArgc, const char** pcArgv){
	QWORD qwTaskID;
	QWORD qwPriority;
	if(iArgc != 3 ){
		kPrintf("%s\n","Wrong Parmeter..");
	}
	kAToI(pcArgv[1]+2, 16,&qwTaskID);
	kAToI(pcArgv[2], 10,&qwPriority);
	kPrintf("Change Task Prioriy ID [0x%q] Priority[%d] ",qwTaskID,(BYTE)qwPriority);
	if(kChangePriority(qwTaskID, (BYTE)qwPriority)==TRUE){
		kPrintf("Success\n");
	}else{
		kPrintf("Fail\n");
	}
}
static void kShowTaskList( int iArgc, const char** pcArgv){
	int i;
	TCB* pstTCB;
	int iCount = 1;
	kPrintf("=============== Task Total Count [%d] ===================\n",kGetTaskCount());
	for(i = 0; i<TASK_MAXCOUNT; i++){
		pstTCB = kGetTCBInTCBPool(i);
		if(((pstTCB->stLinkedList.Node_ID)>>32)!= 0){
			if(iCount % 10 == 0){
				kPrintStringXY(0, CONSOLE_HEIGHT-1, "Press and key to continue...('q' is exit) : ");
				if(kGetCh()=='q'){
					kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");
					break;
				}
				kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");

			}
			kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n",\
					iCount++,pstTCB->stLinkedList.Node_ID,\
					GETPRIORITY(pstTCB->qwFlags),pstTCB->qwFlags);
		}
	}
}
static void kKillTask( int iArgc, const char** pcArgv){
	QWORD qwID;
	TCB* pstTCB;
	int i;
	if(iArgc != 2 ){
			kPrintf("%s\n","Wrong Parmeter..");
	}
	if(kMemCmp((pcArgv[1]),"0x",2) == 0){
		kAToI((pcArgv[1]) + 2 ,16,&qwID);
	}
	else{
		kAToI(pcArgv[1], 10,&qwID);
	}
	if(qwID!=0xFFFFFFFF){
		kPrintf("Kill Task ID[0x%q] ",qwID);
		if(kEndTask(qwID)==TRUE){
			kPrintf("Success\n");
		}else{
			kPrintf("Fail\n");
		}
	}else{
		for(i = 2; i < TASK_MAXCOUNT; i++ ){
			pstTCB = kGetTCBInTCBPool(i);
			qwID = pstTCB->stLinkedList.Node_ID;
			if((qwID>>32)!=0){
				kPrintf("Kill Task ID[0x%q] ",qwID);
				if(kEndTask(qwID)==TRUE){
					kPrintf("Success\n");
				}else{
					kPrintf("Fail\n");
				}
			}
		}
	}

}
static void kCPULoad( int iArgc, const char** pcArgv){
	kPrintf("Proccesor Load : %d\n", kGetProcessorLoad());
}
//================================================================================
void kAutoComplete(char* vcCommandBuffer, int* iCommandBufferIndex){
	int i, iResult, iLastCommandIndex;
	int iLastCommandLength;
	char* pcCandidateCommand;
	int idistance;

	iResult=0;
	vcCommandBuffer[*iCommandBufferIndex] = '\0';
	for(iLastCommandIndex = (*iCommandBufferIndex)-1;iLastCommandIndex>0;iLastCommandIndex--){
		if(vcCommandBuffer[iLastCommandIndex]==' '){
			iLastCommandIndex++;
			break;
		}
	}
	iLastCommandIndex = (iLastCommandIndex == -1)? 0:iLastCommandIndex;
	iLastCommandLength = kStrlen(vcCommandBuffer+iLastCommandIndex);
	//kPrintf("iLastCommand:%s Count:%d\n",vcCommandBuffer+iLastCommandIndex,iLastCommandIndex);
	for(i=0;i<sizeof(gs_vstCommandTable)/sizeof(SHELLCOMMANDENTRY);i++){
		if(iLastCommandLength <= kStrlen(gs_vstCommandTable[i].pcCommand)){
			if(!kMemCmp(&(vcCommandBuffer[iLastCommandIndex]), gs_vstCommandTable[i].pcCommand, iLastCommandLength)){
				pcCandidateCommand = gs_vstCommandTable[i].pcCommand;
				iResult++;
			}
		}
	}
	if(iResult == 1){
		idistance=kMemCpy(&(vcCommandBuffer[*iCommandBufferIndex]), \
				&(pcCandidateCommand[iLastCommandLength]),\
				kStrlen(pcCandidateCommand)-iLastCommandLength);
		vcCommandBuffer[(*iCommandBufferIndex)+idistance] = '\0';
		kPrintf("%s",&(vcCommandBuffer[*iCommandBufferIndex]));
		(*iCommandBufferIndex) += idistance;
	}
}
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;
static void kPrintNumberTask( int iArgc, const char** pcArgv )
{
	int i,j;
	QWORD qwTickCount;
	qwTickCount = kGetTickCount();
	while((kGetTickCount() - qwTickCount)<50){
		kSchedule();
	}
	for( i = 0; i < 5; i++ ){
		kLock(&(gs_stMutex));
		kPrintf("Task ID [0x%Q] Value[%d]\n",kGetRunningTCB()->stLinkedList.Node_ID,\
				gs_qwAdder);
		gs_qwAdder += 1;
		kUnlock(&(gs_stMutex));
		for(j=0;j<30000;j++);
	}
	qwTickCount = kGetTickCount();
	while((kGetTickCount() - qwTickCount)<1000){
		kSchedule();
	}
	kExitTask();
}
static void kTestMutex( int iArgc, const char** pcArgv )
{
	int i;

	gs_qwAdder = 1;
	kInitializeMutex(&gs_stMutex);
	for( i = 0; i < 3; i++ ){
		kCreateTask(TASK_FLAGS_LOW, (QWORD)kPrintNumberTask);
	}
	kPrintf("Wait Until %d Task End...\n",i);
	kGetCh();
}
