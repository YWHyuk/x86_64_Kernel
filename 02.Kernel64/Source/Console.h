/*
 * Console.h
 *
 *  Created on: 2019. 2. 17.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_CONSOLE_H_
#define __02_KERNEL64_SOURCE_CONSOLE_H_
#include "Types.h"

#define CONSOLE_BACKGROUND_BLACK			0x00
#define CONSOLE_BACKGROUND_BLUE 			0x10
#define CONSOLE_BACKGROUND_GREEN			0x20
#define CONSOLE_BACKGROUND_CYAN 			0x30
#define CONSOLE_BACKGROUND_RED  			0x40
#define CONSOLE_BACKGROUND_MAGENTA			0x50
#define CONSOLE_BACKGROUND_BROWN			0x60
#define CONSOLE_BACKGROUND_WHITE			0x70
#define CONSOLE_BACKGROUND_BLINK			0x80
#define CONSOLE_FOREGROUND_DARKBLACK		0X01
#define CONSOLE_FOREGROUND_DARKBLUE			0x02
#define CONSOLE_FOREGROUND_DARKGREEN		0x03
#define CONSOLE_FOREGROUND_DARKCYAN			0x04
#define CONSOLE_FOREGROUND_DARKRED			0x05
#define CONSOLE_FOREGROUND_DARKBROWN		0x06
#define CONSOLE_FOREGROUND_DARKWHITE		0x07
#define CONSOLE_FOREGROUND_BRIGHTBLACK		0x08
#define CONSOLE_FOREGROUND_BRIGHTBLUE		0x09
#define CONSOLE_FOREGROUND_BRIGHTGREEN		0x0A
#define CONSOLE_FOREGROUND_BRIGHTCYAN		0x0B
#define CONSOLE_FOREGROUND_BRIGHTRED		0x0C
#define CONSOLE_FOREGROUND_BRIGHTMAGENTA	0x0D
#define CONSOLE_FOREGROUND_BRIGHTYELLOW		0x0E
#define CONSOLE_FOREGROUND_BRIGHTWHITE		0x0F

#define CONSOLE_DEFAULTTEXTCOLOR			(CONSOLE_BACKGROUND_BLACK| CONSOLE_FOREGROUND_BRIGHTGREEN)

#define CONSOLE_WIDTH					80
#define CONSOLE_HEIGHT					25
#define CONSOLE_VIDEDMEMORYADDRESS 0xB8000

#define VGA_PORT_INDEX				0x3D4
#define VGA_PORT_DATA				0x3D5
#define VGA_INDEX_UPPERCURSOR		0x0E
#define VGA_INDEX_LOWERCURSOR		0x0F

#pragma pack(push,1)

typedef struct kConsoleManagerStruct{
	int iCurrentPrintOffset;
}CONSOLEMANAGER;
#pragma pack(pop)

void kInitializeConsole(int iX, int iY);
void kSetCursor(int iX,int iY);
void kGetCursor(int* iX,int* iY);
void kPrintf(const char* pcFormatString, ...);
int kConsolePrintString(const char* pcBuffer);
void kClearScreen(void);
BYTE kGetCh(void);
/*
 * kGetCh 블락된 영역에서 사용유의, 인터럽트가 차단된 체로 태스크 전환이 일어날 수 있고, 예기치 않은 위험을 초래..
 */
void kPrintStringXY(int iX, int iY, const char* pcString);
#endif /* __02_KERNEL64_SOURCE_CONSOLE_H_ */
