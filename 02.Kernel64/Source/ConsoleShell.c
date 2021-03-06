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
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "MPConfigurationTable.h"
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
		{"testmutex", "Test Mutex Function",kTestMutex},
		{"testthread","Test Thread And Process Function",kTestThread},
		{"showmatrix","Show Matrix Screen",kShowMatrix},
		{"testpie","Test PIE Calculation",kTestPIE},
		{"dynamicmeminfo","Show Dynamic Memory Information",kShowDynamicMemoryInformation},
		{"testranalloc","Test Random Allocation & Free",kTestRandomAllocation},
		{"garbagecollect","",garbage_collect},
		{"hddinfo","Show HDD Information",kShowHDDInformation},
		{"readsector","Read HDD Sector, ex)readsector 0(LBA) 10(count)",kReadSector},
		{"writesecotr","Write HDD Sector, ex)writesector 0(LBA) 10(count)",kWriteSector},
		{"mounthdd","Mount HDD",kMountHDD},
		{"formathdd","Format HDD",kFormatHDD},
		{"filesysteminfo","Show File System Information",kShowFileSystemInformation},
		{"createfile","Create File, Ex)createfile a.txt",kCreateFileInRootDirectory},
		{"deletefile","Delete File, Ex)deletefile a.txt",kDeleteFileInRootDirectory},
		{"dir","Show Directory",kShowRootDirectory},
		{"writefile","Write Data To File, ex) writefile a.txt",kWriteDataToFile},
		{"readfile","Read Data From File, ex) readfile a.txt",kReadDataFromFile},
		{"testfileio","Test File I/O Function",kTestFileIO},
		{"showmpinfo","Show MP Configuration Table Information",kShowMPConfigurationTable},
		{"startap","start application processor", kStartApplicationProcessor},
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
	pfFunction(iArgc,(const char**)ppcArgv);
}
//void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter){}
//int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter){}
//실제 커맨드 함수
static void kHelp( int iArgc, const char** pcArgv ){
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
		if((i % 10 == 0 ) && (i != 0)){
			kPrintStringXY(0, CONSOLE_HEIGHT-1, "Press and key to continue...('q' is exit) : ");
			if(kGetCh()=='q'){
				kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");
				break;
			}
			kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");

		}
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
			if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask1)==NULL)
				break;
		}
		kPrintf("Task1 %d Created\n",i);
		break;
	case 2:
		kAToI(pcArgv[2], 10,&lValue);
		for(i=0;i<lValue;i++){
			if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask2)==NULL)
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
			kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n",\
					iCount++,pstTCB->stLinkedList.Node_ID,\
					GETPRIORITY(pstTCB->qwFlags),pstTCB->qwFlags,count(&(pstTCB->stChildThreadList)));
			kPrintf("Parent PID[0x%q], Memory Address[0x%q], Size[0x%q]\n",\
					pstTCB->qwParentTaskID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize);

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
		pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));
		qwID = pstTCB->stLinkedList.Node_ID;
		if(((qwID>>32)!=0 ) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM)==0)){
			kPrintf("Kill Task ID[0x%q] ",qwID);
			if(kEndTask(qwID)==TRUE){
				kPrintf("Success\n");
			}else{
				kPrintf("Fail\n");
			}
		}else{
			kPrintf("Task does not exist or task is system task\n");
		}
	}else{
		for(i = 0; i < TASK_MAXCOUNT; i++ ){
			pstTCB = kGetTCBInTCBPool(i);
			qwID = pstTCB->stLinkedList.Node_ID;
			if(((qwID>>32)!=0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM)==0)){
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
		kCreateTask(TASK_FLAGS_LOW|TASK_FLAGS_THREAD, 0, 0, (QWORD)kPrintNumberTask);
	}
	kPrintf("Wait Until %d Task End...\n",i);
	kGetCh();
}
static void kCreateThreadTask(void)
{
	int i;
	for(i = 0; i < 3; i++){
		kCreateTask(TASK_FLAGS_LOW|TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask2);
	}
	while(1){
		kSleep(1);
	}
}
static void kTestThread(int iArgc, const char** pcArgv)
{
	TCB* pstProcess;
	pstProcess = kCreateTask(TASK_FLAGS_LOW| TASK_FLAGS_PROCESS, (void*)0xEEEEEEEE, 0x1000, \
			(QWORD)kCreateThreadTask);
	if(pstProcess != NULL){
		kPrintf("Process [0x%q] Create Success\n", pstProcess->stLinkedList.Node_ID);
	}else{
		kPrintf("Process Create Fail\n");
	}
}
static volatile QWORD gs_qwRandomValue = 0;
QWORD kRandom( void )
{
	gs_qwRandomValue = (gs_qwRandomValue * 412151 + 5571031) >> 16;
	return gs_qwRandomValue;
}
void kSetSeed(QWORD qwSeed)
{
	gs_qwRandomValue = qwSeed;
}
static void kDropCharactorThread(void)
{
	int iX, iY;
	int i;
	char vcText[2] = {0,};

	iX = kRandom() % CONSOLE_WIDTH;
	while(1){
		kSleep(kRandom() % 20);
		if((kRandom() % 20)<15){
			vcText[0] = ' ';
			for(i = 0;i < CONSOLE_HEIGHT - 1; i++){
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}

		}else{
			for(i = 0;i < CONSOLE_HEIGHT - 1; i++){
				vcText[ 0 ] = i + kRandom();
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}
		}
	}
}
static void kMatrixProcess(void)
{
	int i;
	for( i = 0; i < 300; i++){
		if(kCreateTask(TASK_FLAGS_THREAD|TASK_FLAGS_LOW, 0, 0, (QWORD)kDropCharactorThread)==NULL){
			break;
		}
		kSleep(kRandom() % 5 + 5);
	}
	kPrintf("%d Thread is created\n",i);
	kGetCh();
}
static void kShowMatrix( int iArgc, const char** pcArgv )
{
	TCB* pstProcess;

	pstProcess = kCreateTask(TASK_FLAGS_PROCESS|TASK_FLAGS_LOW, \
			(void*)0xE00000, 0xE00000, (QWORD)kMatrixProcess);
	if(pstProcess != NULL){
		kPrintf("Matrix Process [0x%q] Create Success\n");
		while((pstProcess->stLinkedList.Node_ID >> 32)!= 0){
			kSleep(100);
		}

	}else{
		kPrintf("Matrix Process Create Fail\n");
	}
}
static void kFPUTestTask(void)
{
	double dValue1;
	double dValue2;
	TCB* pstRunningTask;
	QWORD qwCount = 0;
	QWORD qwRandomValue;
	int i;
	int iOffset;
	char vcData[ 4 ] = {'-','\\','|','/'};
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEDMEMORYADDRESS;

	pstRunningTask = kGetRunningTCB();
	iOffset = (pstRunningTask->stLinkedList.Node_ID & 0xFFFFFFFFUL) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset %(CONSOLE_WIDTH * CONSOLE_HEIGHT));
	while(1){
		dValue1 = 1;
		dValue2 = 1;
		for( i =0;i < 10;i++){
			qwRandomValue = kRandom();
			dValue1 *= (double) qwRandomValue;
			dValue2 *= (double) qwRandomValue;

			kSleep(1);
			qwRandomValue = kRandom();
			dValue1 /= (double) qwRandomValue;
			dValue2 /= (double) qwRandomValue;
		}
		if(dValue1 != dValue2){
			kPrintf("Value is not same [%f] != [%f]\n",dValue1,dValue2);
			break;
		}
		qwCount++;
		pstScreen[iOffset].bCharactor = vcData[qwCount%4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
	}
}
static void kTestPIE( int iArgc, const char** pcArgv )
{
	double dResult;
	int i;
	kPrintf("PIE Calculation Test\n");
	kPrintf("Result: 355 / 133 = ");
	dResult = (double)355/ 113;
	kPrintf("%d.%d%d\n",(QWORD)dResult, ((QWORD)(dResult* 10)%10),((QWORD)(dResult*100)%10));

	for(i = 0; i < 1000; i++ ){
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kFPUTestTask);
	}
}
void kShowDynamicMemoryInformation( int iArgc, const char** pcArgv )
{
	QWORD qwRemain=0;
	QWORD qwUsed=0;
	QWORD qwTotal=0;
	QWORD qwFreeChunknum, qwInUseChunknum;
	FREECHUNK* pstFreeChunk;
	LINKEDLIST* pstLinkedList;
	qwFreeChunknum= count(&(gs_stChunkManager.Free_Chunk_LinkedListManager));
	qwInUseChunknum= count(&(gs_stChunkManager.Chunk_LinkedListManager)) - qwFreeChunknum;

	Print_LinkedList(&(gs_stChunkManager.Chunk_LinkedListManager),Print_InUse_Chunk);
	Print_LinkedList(&(gs_stChunkManager.Free_Chunk_LinkedListManager),Print_Free_Chunk);

	for(pstLinkedList = front(&(gs_stChunkManager.Chunk_LinkedListManager));\
	pstLinkedList!=NULL; pstLinkedList = get_next_node(pstLinkedList)){
		pstFreeChunk = (FREECHUNK*) GETCHUNKADDRESS(pstLinkedList);
		if(pstFreeChunk->qwMagic == CHUNK_TYPE_FREE)
			qwRemain += pstFreeChunk->qwSize;
		else
			qwUsed	+= pstFreeChunk->qwSize;
	//	kPrintf("%q\t",pstLinkedList);
	}
	kPrintf("Alloced Memory: [%d], Remain Memory: [%d], Totla Memory: [%d]\n",qwUsed,qwRemain,qwRemain+qwUsed);
	kPrintf("Used Chunk: [%d], Free Chunk: [%d], Sum Memory: [%d]\n",qwInUseChunknum*sizeof(INUSECHUNK),\
			qwFreeChunknum*sizeof(FREECHUNK),qwInUseChunknum*sizeof(INUSECHUNK)+qwFreeChunknum*sizeof(FREECHUNK));
	kPrintf("All memory: [%d]\n",qwRemain+qwUsed+qwInUseChunknum*sizeof(INUSECHUNK)+qwFreeChunknum*sizeof(FREECHUNK));
}
static void kRandomAllocationTask( void )
{
	TCB* pstTask;
	QWORD qwMemortySize;
	char vcBufferr[ 200 ];
	BYTE* pbAllocationBuffer;
	int i,j;
	int iY;
	pstTask = kGetRunningTCB();
	iY = (pstTask->stLinkedList.Node_ID) % 15 + 9;
	for(j = 0; j < 10; j++ ){
		do{
			qwMemortySize = ((kRandom() % (32 * 1024))+1) * 1024;
			pbAllocationBuffer = kMalloc(qwMemortySize);
			if(pbAllocationBuffer == 0){
				kSleep(1);
				//kPrintf("[0x%q] I'm waiting...\n",kGetRunningTCB());
			}
		}while(pbAllocationBuffer == 0);
		kSPrintf(vcBufferr, "|Address: [0x%Q] Size:[0x%Q] Allocation Success",pbAllocationBuffer,qwMemortySize);
		kPrintStringXY(20, iY, vcBufferr);
		kSleep(200);

		kSPrintf(vcBufferr, "|Address: [0x%Q] Size:[0x%Q] Data write          ",\
				pbAllocationBuffer, qwMemortySize);
		kPrintStringXY(20, iY, vcBufferr);
		for(i = 0; i < qwMemortySize / 2; i++ ){
			pbAllocationBuffer[i] = kRandom() % 0xFF;
			pbAllocationBuffer[i + (qwMemortySize/2)] = pbAllocationBuffer[i];
		}
		kSleep(200);

		kSPrintf(vcBufferr, "|Address: [0x%Q] Size:[0x%Q] Data verify          ",pbAllocationBuffer,qwMemortySize);
		kPrintStringXY(20, iY, vcBufferr);
		for(i = 0; i < qwMemortySize / 2; i++ ){
			if(	pbAllocationBuffer[i] != pbAllocationBuffer[i + (qwMemortySize/2)]){
				kPrintf("Task ID[0x%Q] Verify Fail\n",pstTask->stLinkedList.Node_ID);
				kExitTask();
			}
		}
		kFree(pbAllocationBuffer);
		kSleep(200);
	}
	kSPrintf(vcBufferr, "Task[0x%q] Done",kGetRunningTCB());
	kPrintStringXY(0, iY, vcBufferr);
	kExitTask();
}
static void kTestRandomAllocation( int iArgc, const char** pcArgv )
{

	int i;
	BYTE* temp[3];
	/*
	for(i=0;i<3;i++){
		temp[i] = kMalloc(1);
	}
	kFree(temp[2]);
	kFree(temp[0]);
	kFree(temp[1]);
	kGetCh();
	while(1);
	*/
	kClearScreen();
	for( i = 0; i< 100; i++){
		kCreateTask(TASK_FLAGS_LOWEST|TASK_FLAGS_THREAD, (void*)0, 0, (QWORD)kRandomAllocationTask);
	}
}
static void kShowHDDInformation( int iArgc, const char** pcArgv )
{
	HDDINFORMATION stHDD;
	char vcBuffer[ 100 ];

	if(kReadHDDInformation(TRUE, TRUE, &stHDD) == FALSE){
		kPrintf("HDD Information Read Fail\n");
		return;
	}
	kPrintf("============== Primary Master HDD Information ===================\n");
	kMemCpy(vcBuffer, stHDD.vwModelNumber, sizeof(stHDD.vwModelNumber));
	vcBuffer[sizeof(stHDD.vwModelNumber)-1] = '\0';
	kPrintf("Model Number:\t %s\n",vcBuffer);

	kMemCpy(vcBuffer, stHDD.vwSerialNumber, sizeof(stHDD.vwSerialNumber));
	vcBuffer[sizeof(stHDD.vwSerialNumber)-1] = '\0';
	kPrintf("Serial Number:\t %s\n",vcBuffer);

	kPrintf("Head Count:\t %d\n",stHDD.wNumberOfHead);
	kPrintf("Cylinder Count:\t %d\n",stHDD.wNumberOfCylinder);
	kPrintf("Sector Count:\t %d\n",stHDD.wNumberOfSectorPerCylinder);

	kPrintf("Total Sector:\t %d Sector, %dMB\n",stHDD.dwTotalSector, stHDD.dwTotalSector>>11);

}
static void kReadSector( int iArgc, const char** pcArgv )
{
	QWORD dwLBA;
	BOOL bExit;
	QWORD iSectorCount;
	char* pcBuffer;
	int i,j;
	BYTE bData;
	if(iArgc != 3 ){
		kPrintf("%s\n","Wrong Parmeter..");
		return;
	}
	kAToI(pcArgv[1], 10, &dwLBA);
	kAToI(pcArgv[2], 10, &iSectorCount);

	pcBuffer = kMalloc(iSectorCount * 512);
	if(kReadHDDSector(TRUE, TRUE , dwLBA, iSectorCount, pcBuffer)==iSectorCount){
		kPrintf("LBA [%d], [%d] Sector Read Success",dwLBA,iSectorCount);
		for(j=0;j<iSectorCount;j++){
			for(i=0;i<512;i++){
				if(!((j==0)&&(i==0))&&((i%256)==0)){
					kPrintf("\nPress any Key to continue...");
					if(kGetCh() == 'q'){
						bExit=TRUE;
						break;
					}

				}
				if((i%16)==0)
					kPrintf("\n[LBA:%d, offeset:%d]\t ",dwLBA+j,i);
				bData = pcBuffer[j*512+i] &0xFF;
				if(bData<16)
					kPrintf("0");
				kPrintf("%X ",bData);
			}
			if(bExit == TRUE){
				break;
			}
		}
		kPrintf("\n");

	}else
		kPrintf("Read Fail\n");
	kFree(pcBuffer);
}
static void kWriteSector( int iArgc, const char** pcArgv )
{
	QWORD dwLBA;
	BOOL bExit;
	QWORD iSectorCount;
	char* pcBuffer;
	int i,j;
	BYTE bData;
	static DWORD s_dwWriteCount = 0;
	if(iArgc != 3 ){
		kPrintf("%s\n","Wrong Parmeter..");
		return;
	}
	kAToI(pcArgv[1], 10, &dwLBA);
	kAToI(pcArgv[2], 10, &iSectorCount);
	s_dwWriteCount++;
	pcBuffer = kMalloc(iSectorCount * 512);
	for(j=0;j<iSectorCount;j++){
				for(i=0;i<512/2;i++){
					*(DWORD*) &(pcBuffer[j*512 + i]) = dwLBA + j;
					*(DWORD*) &(pcBuffer[j*512 + i + 4]) = s_dwWriteCount;
				}
	}
	if(kWriteHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer)!=iSectorCount){
		kPrintf("write Fail\n");
		kFree(pcBuffer);
		return;
	}
	if(kReadHDDSector(TRUE, TRUE , dwLBA, iSectorCount, pcBuffer)==iSectorCount){
		kPrintf("LBA [%d], [%d] Sector Read Success",dwLBA,iSectorCount);
		for(j=0;j<iSectorCount;j++){
			for(i=0;i<512;i++){
				if(!((j==0)&&(i==0))&&((i%256)==0)){
					kPrintf("\nPress any Key to continue...");
					if(kGetCh() == 'q'){
						bExit=TRUE;
						break;
					}

				}
				if((i%16)==0)
					kPrintf("\n[LBA:%d, offeset:%d]\t ",dwLBA+j,i);
				bData = pcBuffer[j*512+i] &0xFF;
				if(bData<16)
					kPrintf("0");
				kPrintf("%X ",bData);
			}
			if(bExit == TRUE){
				break;
			}
		}
		kPrintf("\n");

	}
	//kFree(pcBuffer);
}
static void kMountHDD( int iArgc, const char** pcArgv )
{
	if(kMount() ==FALSE){
		kPrintf("HDD Mount Fail...\n");
		return;
	}
	kPrintf("HDD Mount Success\n");
}
static void kFormatHDD( int iArgc, const char** pcArgv )
{
	if(kFormat()==FALSE){
		kPrintf("HDD Format Fail...\n");
		return;
	}
	kPrintf("HDD Format Success\n");
}
static void kShowFileSystemInformation( int iArgc, const char** pcArgv )
{
	FILESYSTEMMANAGER* pstFSInformation;
	pstFSInformation = kMalloc(sizeof(FILESYSTEMMANAGER));
	kGetFileSystemInformation(pstFSInformation);
	kPrintf("======================== File System Information ====================\n");
	kPrintf("Mounted:\t\t\t\t %d\n",pstFSInformation->bMounted);
	kPrintf("Reserved Sector Count: \t\t\t %d Sector\n",pstFSInformation->dwReservedSectorCount);
	kPrintf("Cluster Link Table Start Address:\t %d Sector\n",pstFSInformation->dwClusterLinkAreaStartAddress);
	kPrintf("Data Area Start Address:\t\t %d Sector\n",pstFSInformation->dwDataAreaStartAddress);
	kPrintf("Total Cluster Count:\t\t\t %d Cluster\n",pstFSInformation->dwTotalClusterCount);
	kFree(pstFSInformation);
}
static void kCreateFileInRootDirectory( int iArgc, const char** pcArgv )
{
	int iLength, i;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	FILE* pstFile;
	if(iArgc != 2 ){
		kPrintf("%s\n","Wrong Parmeter..");
		return;
	}
	iLength = kStrlen(pcArgv[1]);
	if((iLength > sizeof(stEntry.vcFileName) )|| (iLength == 0)){
		kPrintf("Too Long name or Too Short File name\n");
		return;
	}
	pstFile = fopen(pcArgv[1],"w");
	if(pstFile == NULL){
		kPrintf("File Create Fail\n");
		return;
	}
	fclose(pstFile);
	kPrintf("File Create success\n");
}
static void kDeleteFileInRootDirectory( int iArgc, const char** pcArgv )
{
	int iLength, iOffset;
	DIRECTORYENTRY stEntry;
	if(iArgc != 2 ){
		kPrintf("%s\n","Wrong Parmeter..");
		return;
	}
	iLength = kStrlen(pcArgv[1]);
	if((iLength > sizeof(stEntry.vcFileName) )|| (iLength == 0)){
		kPrintf("Too Long name or Too Short File name\n");
		return;
	}
	if(remove(pcArgv[1])!=0){
		kPrintf("File Not Found or File Opended\n");
		return;
	}
	kPrintf("File Delete Success\n");
}
static void kShowRootDirectory( int iArgc, const char** pcArgv )
{
	DIR* pstDirectory;
	int i,iCount, iTotalCount;
	struct dirent* pstEntry;
	char vcBuffer[400];
	char vcTempValue[50];
	DWORD dwTotalByte;
	DWORD dwUsedClusterCount;
	FILESYSTEMMANAGER stManager;

	kGetFileSystemInformation(&stManager);
	pstDirectory = opendir("/");
	if(pstDirectory==NULL){
		kPrintf("Root Directory Open Failed\n");
		return;
	}
	iTotalCount = 0;
	dwTotalByte = 0;
	dwUsedClusterCount = 0;
	while(1){
		pstEntry = readdir(pstDirectory);
		if(pstEntry ==NULL){
			break;
		}
		iTotalCount++;
		dwTotalByte += pstEntry->dwFileSize;
		if(pstEntry->dwFileSize==0)
			dwUsedClusterCount++;
		else
			dwUsedClusterCount+=(pstEntry->dwFileSize+(FILESYSTEM_CLUSTERSIZE-1))/FILESYSTEM_CLUSTERSIZE;
	}
	rewinddir(pstDirectory);
	iCount = 0;
	while(1){
		pstEntry = readdir(pstDirectory);
		if(pstEntry == NULL )
			break;
		kMemSet(vcBuffer, ' ', sizeof(vcBuffer)-1);
		vcBuffer[sizeof(vcBuffer)-1]= '\0';
		kMemCpy(vcBuffer, pstEntry->vcFileName, kStrlen(pstEntry->vcFileName));
		kSPrintf(vcTempValue, "%d Byte", pstEntry->dwFileSize);
		kMemCpy(vcBuffer + 30, vcTempValue, kStrlen(vcTempValue));

		kSPrintf(vcTempValue, "0x%X cluser",pstEntry->dwStartClusterIndex);
		kMemCpy(vcBuffer + 55, vcTempValue, kStrlen(vcTempValue)+1);
		kPrintf("     %s\n",vcBuffer);

		if((iCount!=0)&&((iCount %20)==0)){
			kPrintf("Press any key to continue...('q' is exit): ");
			if(kGetCh()=='q'){
				kPrintf("\n");
				break;
			}
		}
		iCount++;
	}
	kPrintf("\t\tTotal File Count: %d\n",iTotalCount);
	kPrintf("\t\tTotal File Size: %d KByte (%d Cluster)\n",dwTotalByte / 1024,dwUsedClusterCount);
	kPrintf("\t\tFree Space: %d KByte (%d Cluster)\n",\
			(stManager.dwTotalClusterCount - dwUsedClusterCount) *FILESYSTEM_CLUSTERSIZE/1024,\
			stManager.dwTotalClusterCount - dwUsedClusterCount);
	closedir(pstDirectory);
}
static void kWriteDataToFile( int iArgc, const char** pcArgv )
{
	int iLength;
	int iEnterCount;
	BYTE bKey;
	FILE* fp;
	iLength = kStrlen(pcArgv[1]);
	if((iLength >FILESYSTEM_MAXFILENAMELENGTH - 1 )|| (iLength == 0)){
		kPrintf("Too Long name or Too Short File name\n");
		return;
	}
	fp = fopen(pcArgv[1],"w");
	if(fp==NULL){
		kPrintf("File Open Failed\n");
		return;
	}
	iEnterCount = 0;
	while(1){
		bKey = kGetCh();
		if(bKey == KEY_ENTER){
			iEnterCount++;
			if(iEnterCount>=3)
				break;
		}else{
			iEnterCount=0;
		}
		kPrintf("%c",bKey);
		if(fwrite(&bKey,1,1,fp)!=1){
			kPrintf("File Write Fail\n");
			break;
		}
	}
	kPrintf("File Write Success\n");
	fclose(fp);
}
static void kReadDataFromFile( int iArgc, const char** pcArgv ){
	int iLength;
	int iEnterCount;
	BYTE bKey;
	FILE* fp;
	iLength = kStrlen(pcArgv[1]);
	if((iLength >FILESYSTEM_MAXFILENAMELENGTH - 1 )|| (iLength == 0)){
		kPrintf("Too Long name or Too Short File name\n");
		return;
	}
	fp = fopen(pcArgv[1],"r");
	if(fp==NULL){
		kPrintf("File Open Failed\n");
		return;
	}
	iEnterCount = 0;
	while(1){
		if(fread(&bKey,1,1,fp)!=1){
			break;
		}
		kPrintf("%c",bKey);
		if(bKey == KEY_ENTER){
			iEnterCount++;
			if((iEnterCount!=0)&&(iEnterCount%20==0)){
				kPrintf("Press any key to continue... ('q' is exit) : ");
				if(kGetCh()=='q'){
					kPrintf("\n");
					break;
				}
				kPrintf("\n");
				iEnterCount = 0;
			}
		}
	}
	fclose(fp);
}
/**
 *  파일 I/O에 관련된 기능을 테스트
 */
static void kTestFileIO( int iArgc, const char** pcArgv )
{
    FILE* pstFile;
    BYTE* pbBuffer;
    int i;
    int j;
    DWORD dwRandomOffset;
    DWORD dwByteCount;
    BYTE vbTempBuffer[ 1024 ];
    DWORD dwMaxFileSize;

    kPrintf( "================== File I/O Function Test ==================\n" );

    // 4Mbyte의 버퍼 할당
    dwMaxFileSize = 4 * 1024 * 1024;
    pbBuffer = kMalloc( dwMaxFileSize );
    if( pbBuffer == NULL )
    {
        kPrintf( "Memory Allocation Fail\n" );
        return ;
    }
    // 테스트용 파일을 삭제
    remove( "testfileio.bin" );

    //==========================================================================
    // 파일 열기 테스트
    //==========================================================================
    kPrintf( "1. File Open Fail Test..." );
    // r 옵션은 파일을 생성하지 않으므로, 테스트 파일이 없는 경우 NULL이 되어야 함
    pstFile = fopen( "testfileio.bin", "r" );
    if( pstFile == NULL )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
        fclose( pstFile );
    }
    kGetCh();

    //==========================================================================
    // 파일 생성 테스트
    //==========================================================================
    kPrintf( "2. File Create Test..." );
    // w 옵션은 파일을 생성하므로, 정상적으로 핸들이 반환되어야함
    pstFile = fopen( "testfileio.bin", "w" );
    if( pstFile != NULL )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    kGetCh();
    //==========================================================================
    // 순차적인 영역 쓰기 테스트
    //==========================================================================
    kPrintf( "3. Sequential Write Test(Cluster Size)..." );
    // 열린 핸들을 가지고 쓰기 수행
    BYTE *bb=(BYTE*)kMalloc(FILESYSTEM_CLUSTERSIZE);
    for( i = 0 ; i < 100 ; i++ )
    {
    	//kPrintf("before index: %d\n offset:%d\n",pstFile->stFileHandle.dwCurrentClusterIndex,\
    			pstFile->stFileHandle.dwCurrentOffset);
        kMemSet( pbBuffer, i, FILESYSTEM_CLUSTERSIZE );
        if( fwrite( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) !=
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    %d Cluster Error\n", i );
            break;
        }
    }
    if( i >= 100 )
    {
        kPrintf( "[Pass]\n" );
    }
    kGetCh();
    //==========================================================================
    // 순차적인 영역 읽기 테스트
    //==========================================================================
    kPrintf( "4. Sequential Read And Verify Test(Cluster Size)..." );
    // 파일의 처음으로 이동

    if(fseek( pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_END )!=0)
    	return;
    // 열린 핸들을 가지고 읽기 수행 후, 데이터 검증
    for( i = 0 ; i < 100 ; i++ )
    {
        // 파일을 읽음
        if( fread( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) !=
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }
        // 데이터 검사
        for( j = 0 ; j < FILESYSTEM_CLUSTERSIZE ; j++ )
        {
            if( pbBuffer[ j ] != ( BYTE ) i )
            {
                kPrintf( "[Fail]\n" );
                kPrintf( "    %d Cluster Error. [%X] != [%X]\n", i, pbBuffer[ j ],
                         ( BYTE ) i );
                return;
            }
        }
    }
    if( i >= 100 )
    {
        kPrintf( "[Pass]\n" );
    }
    kGetCh();
    //==========================================================================
    // 임의의 영역 쓰기 테스트
    //==========================================================================
    kPrintf( "5. Random Write Test...\n" );

    // 버퍼를 모두 0으로 채움
    kMemSet( pbBuffer, 0, dwMaxFileSize );
    // 여기 저기에 옮겨다니면서 데이터를 쓰고 검증
    // 파일의 내용을 읽어서 버퍼로 복사
    if(fseek( pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_CUR )!=0){
    	kPrintf("fseek failed\n");
    	return;
    }
    fread( pbBuffer, 1, dwMaxFileSize, pstFile );

    // 임의의 위치로 옮기면서 데이터를 파일과 버퍼에 동시에 씀
    for( i = 0 ; i < 100 ; i++ )
    {
        dwByteCount = ( kRandom() % ( sizeof( vbTempBuffer ) - 1 ) ) + 1;
        dwRandomOffset = kRandom() % ( dwMaxFileSize - dwByteCount );

        kPrintf( "    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
                dwByteCount );

        // 파일 포인터를 이동
        if(fseek( pstFile, dwRandomOffset, SEEK_SET )!=0)
        	return;
        kMemSet( vbTempBuffer, i, dwByteCount );

        // 데이터를 씀
        if( fwrite( vbTempBuffer, 1, dwByteCount, pstFile ) != dwByteCount )
        {
            kPrintf( "[Fail]\n" );
            break;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }

        kMemSet( pbBuffer + dwRandomOffset, i, dwByteCount );
    }

    // 맨 마지막으로 이동하여 1바이트를 써서 파일의 크기를 4Mbyte로 만듦
    int temp=fseek( pstFile, dwMaxFileSize - 1, SEEK_SET );
    if(temp!=0)
    	kPrintf("fseek() failed...%d\n",temp);
    fwrite( &i, 1, 1, pstFile );
    pbBuffer[ dwMaxFileSize - 1 ] = ( BYTE ) i;
    kGetCh();
    //==========================================================================
    // 임의의 영역 읽기 테스트
    //==========================================================================
    kPrintf( "6. Random Read And Verify Test...\n" );
    // 임의의 위치로 옮기면서 파일에서 데이터를 읽어 버퍼의 내용과 비교
    for( i = 0 ; i < 100 ; i++ )
    {
        dwByteCount = ( kRandom() % ( sizeof( vbTempBuffer ) - 1 ) ) + 1;
        dwRandomOffset = kRandom() % ( ( dwMaxFileSize ) - dwByteCount );

        kPrintf( "    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
                dwByteCount );

        // 파일 포인터를 이동
        if(fseek( pstFile, dwRandomOffset, SEEK_SET )!=0)
        	return;

        // 데이터 읽음
        if( fread( vbTempBuffer, 1, dwByteCount, pstFile ) != dwByteCount )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    Read Fail\n", dwRandomOffset );
            break;
        }

        // 버퍼와 비교
        if( kMemCmp( pbBuffer + dwRandomOffset, vbTempBuffer, dwByteCount )
                != 0 )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    Compare Fail\n", dwRandomOffset );
            break;
        }

        kPrintf( "[Pass]\n" );
    }
    kGetCh();
    //==========================================================================
    // 다시 순차적인 영역 읽기 테스트
    //==========================================================================
    kPrintf( "7. Sequential Write, Read And Verify Test(1024 Byte)...\n" );
    // 파일의 처음으로 이동
    if(fseek( pstFile, -dwMaxFileSize, SEEK_CUR )!=0)
    	return;

    // 열린 핸들을 가지고 쓰기 수행. 앞부분에서 2Mbyte만 씀
    for( i = 0 ; i < ( 2 * 1024 * 1024 / 1024 ) ; i++ )
    {
        kPrintf( "    [%d] Offset [%d] Byte [%d] Write...", i, i * 1024, 1024 );

        // 1024 바이트씩 파일을 씀
        if( fwrite( pbBuffer + ( i * 1024 ), 1, 1024, pstFile ) != 1024 )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }
    }

    // 파일의 처음으로 이동
   if(fseek( pstFile, -dwMaxFileSize, SEEK_SET )!=0)
	   return;

    // 열린 핸들을 가지고 읽기 수행 후 데이터 검증. Random Write로 데이터가 잘못
    // 저장될 수 있으므로 검증은 4Mbyte 전체를 대상으로 함
    for( i = 0 ; i < ( dwMaxFileSize / 1024 )  ; i++ )
    {
        // 데이터 검사
        kPrintf( "    [%d] Offset [%d] Byte [%d] Read And Verify...", i,
                i * 1024, 1024 );

        // 1024 바이트씩 파일을 읽음
        if( fread( vbTempBuffer, 1, 1024, pstFile ) != 1024 )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }

        if( kMemCmp( pbBuffer + ( i * 1024 ), vbTempBuffer, 1024 ) != 0 )
        {
            kPrintf( "[Fail]\n" );
            break;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }
    }
    kGetCh();
    //==========================================================================
    // 파일 삭제 실패 테스트
    //==========================================================================
    kPrintf( "8. File Delete Fail Test..." );
    // 파일이 열려있는 상태이므로 파일을 지우려고 하면 실패해야 함
    if( remove( "testfileio.bin" ) != 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    kGetCh();
    //==========================================================================
    // 파일 닫기 테스트
    //==========================================================================
    kPrintf( "9. File Close Test..." );
    // 파일이 정상적으로 닫혀야 함
    if( fclose( pstFile ) == 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    kGetCh();
    //==========================================================================
    // 파일 삭제 테스트
    //==========================================================================
    kPrintf( "10. File Delete Test..." );
    // 파일이 닫혔으므로 정상적으로 지워져야 함
    if( remove( "testfileio.bin" ) == 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }

    // 메모리를 해제
    kFree( pbBuffer );
}
static void kShowMPConfigurationTable( int iArgc, const char** pcArgv )
{
	kPrintMPConfigurationTable();
}
static void kStartApplicationProcessor( int iArgc, const char** pcArgv )
{
	if(kStartUpApplicationProcessor() == FALSE){
		kPrintf("Application Processor Start Fail\n");
		return;
	}
	kPrintf("Application Processor Start Succes\n");

	kPrintf("Bootstrap Processor[APIC ID: %d] Start Application Processor\n",kGetAPICID());
}
