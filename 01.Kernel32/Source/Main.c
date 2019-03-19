#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"
/*
 * 페이징을 하는 세그먼트 및 페이지를 더욱 더 자세하게 기술한다면 안정성이 더 나아지지 않을까?
 *
 */
void kPrintString( int iX, int iY, const char* pcString);
BOOL KInitializeKernel64Area(void);
BOOL KIsMemoryEnough(void);
void kCopyKernel64ImageTo2Mbyte(void);

#define BOOTSTRAPPROCESSOR_FLAGADDRESS	0x7c09

void Main(void){
	DWORD i;
	DWORD dwEAX,dwEBX,dwECX,dwEDX;
	char vcVendorString[13] ={0,};
	if(*((BYTE*)BOOTSTRAPPROCESSOR_FLAGADDRESS)==0){
		kSwitchAndExecute64bitKernel();
		while(1);
	}
	kPrintString(0,3,"C Language Starting...");
	kPrintString(45,3,"[Pass]");
	kPrintString(0, 4, "Minimum Memory Size Check...");
	if(KIsMemoryEnough() == FALSE){
		kPrintString(45, 4, "[Fail]");
		kPrintString(0, 5, "Not Enough Memory... Not Supported this enviorment...");
		goto LOOP;
	}else{
		kPrintString(45, 4, "[Pass]");
	}
	kPrintString(0, 5, "IA-32e Kernel Area Initialize...");
	if(KInitializeKernel64Area()==FALSE){
		kPrintString(45, 5, "[Fail]");
		kPrintString(0, 6, "Kernel Area Initialization Failed...");
		goto LOOP;
	}
	kPrintString(45, 5, "[Pass]");
	kPrintString(0,6,"IA-32e Page Tables Initializing... ");
	kInitializePageTables();
	kPrintString(45,6,"[Pass]");
	kReadCPUID(0x00,&dwEAX,&dwEBX,&dwECX,&dwEDX);
	*((DWORD*) vcVendorString	 )=dwEBX;
	*((DWORD*) vcVendorString + 1)=dwEDX;
	*((DWORD*) vcVendorString + 2)=dwECX;
	kPrintString(0,7,"Processor Vendor String....");
	kPrintString(45,7,vcVendorString);

	kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
	kPrintString(0, 8, "64bit Mode Support Check...");
	if(dwEDX &(1<<29)){
		kPrintString(45, 8, "[Pass]");
	}else{
		kPrintString(45, 8, "[Fail]");
		kPrintString(0, 9, "This Processor does not support 64bit mode...");
	}
	kPrintString(0, 9, "Copy IA-32e Kernel To 2M Address...!");
	//goto LOOP;
	kCopyKernel64ImageTo2Mbyte();
	kPrintString(45,10,"[Pass]");
	kPrintString(0, 10, "Switch To IA-32e Mode");
	kSwitchAndExecute64bitKernel();
LOOP:
	while(TRUE);
}
void kPrintString( int iX, int iY, const char* pcString){
	CHARACTER* pstScreen=(CHARACTER *) 0xB8000;
	pstScreen += (iY * 80) + iX;
	int i;
	for( i=0;pcString[i]!=0;i++){
		pstScreen[i].bCharactor = pcString[i];
	}
}
BOOL KInitializeKernel64Area(void){
	DWORD* pdwCurrentAddress;
	pdwCurrentAddress = (DWORD*) 0x100000;
	while((DWORD)pdwCurrentAddress <0x600000){
		*pdwCurrentAddress = 0x00;
		if(*pdwCurrentAddress!=0){
			return FALSE;
		}
		pdwCurrentAddress++;
	}
	return TRUE;
}
BOOL KIsMemoryEnough(void){
	DWORD* pdwCurrentAddress;
	pdwCurrentAddress = (DWORD*)0x100000;
	while((DWORD)pdwCurrentAddress<0x4000000){
		*pdwCurrentAddress = 0x12345678;
		if(*pdwCurrentAddress !=0x12345678){
			return FALSE;
		}
		pdwCurrentAddress += (0x100000 /4 );
	}
	return TRUE;
}
void kCopyKernel64ImageTo2Mbyte(void){
	WORD wKernel32SectorCount, wTotalKernelSectorCount;
	DWORD* pdwSourceAddress, *pdwDestinationAddress;
	WORD i;
	wTotalKernelSectorCount=*((WORD*)0x7c05);
	wKernel32SectorCount=*((WORD*)0x7c07);
	pdwSourceAddress = (DWORD*)(0x10000 + (wKernel32SectorCount * 512));
	pdwDestinationAddress = (DWORD*)0x200000;
	for(i=0;i<(512*(wTotalKernelSectorCount - wKernel32SectorCount))/4;i++){
		*pdwDestinationAddress = *pdwSourceAddress;
		//kReadMemory(0, (15+i), (DWORD*)0x200000);
		pdwDestinationAddress++;
		pdwSourceAddress++;
	}
}
/*
void kReadMemory(int x,int y,DWORD* address){
	BYTE* buffer=(BYTE*)address;
	int i=0;
	char string[4]={0,0,' ',0};
	for(i=0;i<4;i++){
		HexToChar(buffer[i], string);
		kPrintString(x+i*3, y, string);
	}
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
*/
