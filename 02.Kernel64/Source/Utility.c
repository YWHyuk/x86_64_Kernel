/*
 * Utility.c
 *
 *  Created on: 2019. 2. 12.
 *      Author: DEWH
 */
#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>
void kMemSet(void* pvDestination, BYTE bData, int iSize){
	int i;
	for(i=0;i<iSize;i++){
		((char*) pvDestination)[i]=bData;
	}
}
int kMemCpy(void* pvDestination, const void* pvSource, int iSize){
	int i;
	for(i=0;i<iSize;i++){
		((char*) pvDestination)[i]=((char*) pvSource)[i];
	}
	return iSize;
}
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize){
	int i;
	char cTemp;
	for(i=0;i<iSize;i++){
		cTemp=((char*) pvDestination)[i]-((char*) pvSource)[i];
		if(cTemp!=0){
			return (int) cTemp;
		}
	}
	return 0;
}
int kStrlen(const char* pcBuffer){
	int iReturn=0;
	int i;
	QWORD qwTemp;
	/*
	 * 이거 자기 메모리 밖의 영역을 접근한다는 점이 매우 찜찜함
	 * 마지막 전까지는 뭉터기로 읽고 마지막 잔잔바리는 하나씩 읽는 걸로 수정하자
	 */
	while(TRUE){
		qwTemp = ((QWORD*)(pcBuffer))[iReturn/sizeof(QWORD)];
		for(i=0;i<sizeof(QWORD);i++){
			if((qwTemp & 0xFF) == 0x00)
				return iReturn;
			qwTemp>>=8;
			iReturn++;
		}
	}
}
BOOL kSetInterruptFlag(BOOL bEnableInterrupt){
	QWORD qwRFlags;
	qwRFlags = kReadRFLAGS();
	if(bEnableInterrupt == TRUE){
		kEnableInterrupt();
	}else{
		kDisableInterrupt();
	}
	if(qwRFlags & 0x0200)
		return TRUE;
	return FALSE;
}
static QWORD gs_qwTotalRAMMBSize = 0;
void kCheckTotalRAMSize( void ){
	DWORD* pdwCurrentAddress;
	DWORD dwPreviousValue;
	pdwCurrentAddress = (DWORD*) 0x400000;
	while(1){
		dwPreviousValue = *pdwCurrentAddress;
		*pdwCurrentAddress = 0x12345678;
		if(*pdwCurrentAddress != 0x12345678)
			break;
		*pdwCurrentAddress = dwPreviousValue;
		pdwCurrentAddress += (0x400000/4);

	}
	gs_qwTotalRAMMBSize = (QWORD) pdwCurrentAddress / 0x100000;
}
QWORD kGetTotalRAMSize( void ){
	return gs_qwTotalRAMMBSize;
}
void kReverseString( char* pcBuffer){
	int iBufferLength = kStrlen(pcBuffer);
	int i;
	char cTemp;
	for(i=0;i<iBufferLength/2;i++){
		cTemp=pcBuffer[iBufferLength-1-i];
		pcBuffer[iBufferLength-1-i]= pcBuffer[i];
		pcBuffer[i]=cTemp;
	}
}
long kAToI( const char* pcBuffer, int iRadix)// 문자열을  인자로 준 진수형태로 해석해서 정수형으로 만든다.
{
	int iLengh,i;
	long lReturn=0L;
	char cTemp;
	BOOL bMinus=FALSE;
	if(pcBuffer[0]=='-')
		bMinus=TRUE;

	iLengh= kStrlen(pcBuffer);
	for(i=bMinus;i<iLengh;i++){
		lReturn*= iRadix;
		cTemp= pcBuffer[i];
		if(cTemp>='0' && cTemp<='9'){
			cTemp-= '0';
		}else if(cTemp>='A' && cTemp<='Z'){
			cTemp = cTemp - 'A' + 10;
		}
		else if(cTemp>='a' && cTemp<='F'){
			cTemp = cTemp - 'a' + 10;
		}else{
			return -1;
		}
		if(cTemp>=iRadix)
			return -1;
		lReturn += cTemp;

	}
	if(bMinus ==TRUE)
		lReturn= -lReturn;
	return lReturn;
}
QWORD kHexStringToQword( const char* pcBuffer);//
long kDecimalStringToLong( const char* pcBuffer);
int kIToA( long lValue, char* pcBuffer, int iRadix){
	int iBufferIndex=0;
	long iTemp;
	BOOL bMinus=FALSE;
	if(lValue < 0){
		pcBuffer[iBufferIndex]='-';
		iBufferIndex++;
		lValue *= -1;
		bMinus = TRUE;
	}
	if(lValue == 0){
		pcBuffer[iBufferIndex]= '0';
		iBufferIndex++;
	}
	while(lValue > 0){
		iTemp = lValue % iRadix;
		lValue /= iRadix;
		if(iTemp>=10)
			pcBuffer[iBufferIndex]= 'A'+iTemp-10;
		else
			pcBuffer[iBufferIndex]= '0'+iTemp;
		iBufferIndex++;
	}
	pcBuffer[iBufferIndex]= '\0';
	kReverseString(pcBuffer+bMinus);
	return iBufferIndex;
}
int kHexToString(QWORD qwValue, char* pcBuffer);
int kDecimalToString(long lvalue, char* pcBuffer);
int kSPrintf( char* pcBuffer, const char* pcFormatString, ...	){
	va_list ap;
	int iReturn;
	va_start(ap,pcFormatString);
	iReturn = kVSPrintf(pcBuffer, pcFormatString, ap);
	va_end(ap);
	return iReturn;
}
int kVSPrintf( char* pcBuffer, const char* pcFormatString, va_list ap){
	QWORD i,j;
	int iBufferIndex =0;
	int iFormatLength, iCopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;
	iFormatLength = kStrlen(pcFormatString);
	for(i = 0;i<iFormatLength;i++){
		if(pcFormatString[i]=='%'){
			i++;
			switch(pcFormatString[i])
			{
			case 's'://문자열 출력
				pcCopyString = (char*) (va_arg(ap,char*));
				iValue = kStrlen(pcCopyString);
				kMemCpy(pcBuffer+iBufferIndex, pcCopyString, iValue);
				iBufferIndex+= iValue;
				break;
			case 'c'://문자 출력
				pcBuffer[iBufferIndex] = (char) (va_arg(ap,int));
				iBufferIndex++;
				break;
			case 'd'://정수 출력
			case 'i':
				iValue = (int) (va_arg(ap,int));
				iBufferIndex += kIToA(iValue, pcBuffer+iBufferIndex, 10);
				break;
			case 'x'://4byte Hex 출력
			case 'X':
				qwValue = (DWORD) (va_arg(ap,DWORD)) & 0xFFFFFFFF;
				iBufferIndex += kIToA(qwValue, pcBuffer+iBufferIndex, 16);
				break;
			case 'q'://8byte Hex 출력
			case 'Q':
			case 'p':
				qwValue = (QWORD) (va_arg(ap,QWORD));
				iBufferIndex += kIToA(qwValue, pcBuffer+iBufferIndex, 16);
				break;
			default:
				pcBuffer[iBufferIndex] = pcFormatString[i];
				iBufferIndex++;
				break;
			}
		}else{
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}
	pcBuffer[iBufferIndex] ='\0';
	return iBufferIndex;
}
BOOL kIsPrintable(char cKey){
	BOOL bReturn= FALSE;
	if(cKey >= 0x21 && cKey <= 0x7E )
		bReturn = TRUE;
	return bReturn;
}
volatile QWORD g_qwTickCount = 0;
QWORD kGetTickCount(void){
	return g_qwTickCount;
}
