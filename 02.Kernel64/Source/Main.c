#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Queue.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "Pit.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "MultiProcessor.h"
static MUTEX consoleMutex;
void MainForApplicationProcessor( void );
void Main(void){
		int iCursorX, iCursorY;

		/* AP라면... */
		if(*((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) == 0){
			MainForApplicationProcessor();
		}
		/* BP라면... */
		*((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) = 0;
		kInitializeMutex(&consoleMutex);
		kInitializeConsole(0, 10);

	    kPrintf("Switch To IA-32e Mode Success...\n" );
	    iCursorY++;
	    kPrintf("IA-32e C Language Kernel Start...\n" );
	    kPrintStringXY( 45, iCursorY++, "[Pass]" );

	    kPrintf("GDT Initialize And Switch For IA-32e Mode...\n");
	    kInitializeGDTTableAndTSS();
	    kLoadGDTR(GDTR_STARTADDRESS);
	    kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("TSS Segment Load...\n" );
		kLoadTR(GDT_TSSSEGMENT);
		kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("IDT Initialize...\n" );
	    kInitializeIDTTables();
	    kLoadIDTR(IDTR_STARTADDRESS);
	    kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("Total RAM Size Check...");
	    kCheckTotalRAMSize();
	    kGetCursor(&iCursorX, &iCursorY);
	    kSetCursor(45, iCursorY++);
	    kPrintf("[Pass], Size = %d MB\n",kGetTotalRAMSize());

	    kPrintf("TCB Pool And Scheduler Initialize...\n");
	    kInitializeTCBPool();
	    kInitializeScheduler();
	    kInitializePIT(MSTOCOUNT(1), 1);
	    kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("Dynamic Memory Initialize...\n");
	    kInitializeDynamicMemory();
	    kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("Keyboard Activate\n");
	    kPrintStringXY(45, iCursorY++, "[Pass]");
		//breakfunc();
	    // 키보드를 활성화
	    if( kInitializeKeyboard() == TRUE )
	    {
	        kPrintStringXY( 45, 12, "[Pass]" );
	        kChangeKeyboardLED( FALSE, FALSE, FALSE );
	    }
	    else
	    {
	        kPrintStringXY( 45, 12, "Fail" );
	        while( 1 ) ;
	    }
	    kPrintf("PIC Controller And Interrupt Initilalize...\n");
	    kInitializePIC();
	    kMaskPICInterrupt(0);
	    kEnableInterrupt();
	    kPrintStringXY(45, iCursorY++, "[Pass]");

	    kPrintf("HDD Initialize...\n");
	    if(kInitializeHDD()==TRUE)
	    	kPrintStringXY(45, iCursorY++, "[Pass]");
	    else
	    	kPrintStringXY(45, iCursorY++, "[Fail]");

	    kPrintf("File System Initialize...\n");
	 	    if(kInitializeFileSystem()==TRUE)
	 	    	kPrintStringXY(45, iCursorY++, "[Pass]");
	 	    else
	 	    	kPrintStringXY(45, iCursorY++, "[Fail]");

	    kCreateTask(TASK_FLAGS_LOWEST|TASK_FLAGS_IDLE|TASK_FLAGS_THREAD|TASK_FLAGS_SYSTEM,\
	    		0, 0, (QWORD)kIdleTask);
	    kStartConsoleShell();
LOOP:
	while(TRUE);
}
void MainForApplicationProcessor( void )
{
	QWORD qwTickCount;
	kLoadGDTR(GDTR_STARTADDRESS);
	kLoadTR(GDT_TSSSEGMENT + (kGetAPICID() * sizeof(GDTENTRY16)));

	kLoadIDTR(IDTR_STARTADDRESS);
	char buffer[200];
	qwTickCount = kGetTickCount();
	while(1){
		if(kGetTickCount() - qwTickCount > 1000){
			qwTickCount = kGetTickCount();
			kSPrintf(buffer, "Application Processor[APIC ID: %d] Is Activated\n",kGetAPICID());
			kPrintStringXY(0, kGetAPICID(), buffer);
		}
	}
}
