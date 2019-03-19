/*
 * Console.c
 *
 *  Created on: 2019. 2. 17.
 *      Author: DEWH
 */
#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"

CONSOLEMANAGER gs_stConsoleManager={0,};
void kInitializeConsole(int iX, int iY){
	kMemSet(&gs_stConsoleManager, 0, sizeof(CONSOLEMANAGER));
	kSetCursor(iX, iY);
}
void kSetCursor(int iX,int iY){
	int iLinearValue;
	iLinearValue = iY* CONSOLE_WIDTH + iX;

	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
	kOutPortByte(VGA_PORT_DATA, iLinearValue>>8);

	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
	kOutPortByte(VGA_PORT_DATA, iLinearValue & 0xFF);

	gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;

}
void kGetCursor(int* iX,int* iY){
	(*iX) = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
	(*iY) = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}
void kPrintf(const char* pcFormatString, ...){
	char vcBuffer[100]={0,};
	int iNextPrintOffset;
	va_list ap;
	va_start(ap,pcFormatString);
	kVSPrintf(vcBuffer, pcFormatString,ap);
	va_end(ap);
	iNextPrintOffset = kConsolePrintString(vcBuffer);
	kSetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}
int kConsolePrintString(const char* pcBuffer){
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEDMEMORYADDRESS;
	int i,j;
	int iLengh;
	int iPrintOffset;
	iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

	iLengh = kStrlen(pcBuffer);
	for(i=0;i<iLengh;i++){
		if(pcBuffer[i]=='\t'){
			iPrintOffset += 8 - (iPrintOffset % 8);
		}else if(pcBuffer[i]=='\n'){
			iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
		}else{
			pstScreen[iPrintOffset].bCharactor=pcBuffer[i];
			pstScreen[iPrintOffset].bAttribute=CONSOLE_DEFAULTTEXTCOLOR;
			iPrintOffset++;
		}
		if(iPrintOffset >= (CONSOLE_WIDTH * CONSOLE_HEIGHT)){
			kMemCpy((void*)CONSOLE_VIDEDMEMORYADDRESS + CONSOLE_WIDTH * sizeof(CHARACTER),\
					(void*)CONSOLE_VIDEDMEMORYADDRESS + CONSOLE_WIDTH * sizeof(CHARACTER) * 2,\
					(CONSOLE_HEIGHT - 2)*CONSOLE_WIDTH * sizeof(CHARACTER));
			for( j = (CONSOLE_HEIGHT- 1) *CONSOLE_WIDTH; j<(CONSOLE_HEIGHT * CONSOLE_WIDTH);j++){
				pstScreen[j].bAttribute= CONSOLE_DEFAULTTEXTCOLOR;
				pstScreen[j].bCharactor= ' ';
			}
			iPrintOffset = (CONSOLE_HEIGHT-1) * CONSOLE_WIDTH;
		}
	}
	return iPrintOffset;
}
void kClearScreen(void){
	CHARACTER* pstScreen=(CHARACTER *) 0xB8000;
	int i;
	for( i=0;i<CONSOLE_WIDTH*CONSOLE_HEIGHT;i++){
		pstScreen[i].bCharactor = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
	kSetCursor(0, 1);
}
BYTE kGetCh(void){
	KEYDATA stData;
	while( TRUE )
	{
		// 키 큐에 데이터를 읽기 시도
		if( kGetKeyFromKeyQueue(&stData) == TRUE )
		{
			// 스캔 코드를 ASCII 코드로 변환하는 함수를 호출하여 ASCII 코드와
			// 눌림 또는 떨어짐 정보를 반환
			if(stData.bFlags & KEY_FLAGS_DOWN)
			{
				return stData.bASCIICode;
			}
		}
		kSchedule();
	}
}
void kPrintStringXY(int iX, int iY, const char* pcString){
	CHARACTER* pstScreen=(CHARACTER *) 0xB8000;
	pstScreen += (iY * 80) + iX;
	int i;
	for( i=0;pcString[i]!=0;i++){
		pstScreen[i].bCharactor = pcString[i];
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
}
