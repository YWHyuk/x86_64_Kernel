/*
 * Pit.c
 *
 *  Created on: 2019. 2. 18.
 *      Author: DEWH
 */
#include "Pit.h"

void kInitializePIT(WORD wCount, BOOL bPeriodic){
	/*
	 * 컨트롤 레지스터에 값을 쓰고, 0번 타이머에 값을 쓰자.
	 */
	kOutPortByte(PIT_CONTROL_PORT, PIT_COUNTER0_ONCE);

	if(bPeriodic == TRUE){
		kOutPortByte(PIT_CONTROL_PORT, PIT_COUNTER0_PERIODIC);
	}

	kOutPortByte(PIT_COUNTER0_PORT, wCount & 0xFF);
	kOutPortByte(PIT_COUNTER0_PORT, (wCount >> 8) & 0xFF);
}
WORD kReadCounter0( void ){
	WORD wReturn = 0;
	BYTE bTemp;
	kOutPortByte(PIT_CONTROL_PORT, PIT_CONTROL_RW_2BYTE_READ);
	bTemp = kInPortByte(PIT_COUNTER0_PORT);
	wReturn = bTemp;
	bTemp = kInPortByte(PIT_COUNTER0_PORT);
	wReturn = wReturn | (bTemp << 8);

	return wReturn;
}
void kWaitUsingDirectPIT(WORD wCount){
	WORD wLastCountNumber,wNowCountNumber;
	DWORD dwSpendedCount,dwTotalCount;
	DWORD dwCount=(DWORD)wCount;
	kInitializePIT(0, FALSE);// wCount의 크기는 어짜피 한정되어있고, 반복적으로 하는 것은 의미가 거의 없다.
	wLastCountNumber=kReadCounter0();
	//kPrintf("Start Count:%x\n",wLastCountNumber);
	while(TRUE){
		wNowCountNumber = kReadCounter0();
		if((wLastCountNumber - wNowCountNumber >= wCount) | wNowCountNumber == 0 ){
			//kPrintf("Now Count:%x %x\n",wNowCountNumber,wCount);
			break;
		}
	}
}
void kWaitms(long lMillisecond){
	int i;
	for(i=0;i<lMillisecond/30;i++){
		kWaitUsingDirectPIT(MSTOCOUNT(30));
	}
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
}
