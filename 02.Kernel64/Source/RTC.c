/*
 * RTC.c
 *
 *  Created on: 2019. 2. 18.
 *      Author: DEWH
 */

#include "RTC.h"
void kReadRTCTIME(BYTE* pbHour,BYTE* pbMinute, BYTE* pbSecond){
	BYTE bData;
	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_HOUR);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbHour = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_SECOND);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbSecond = RTC_BCDTOBINARY(bData);

}
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek){
	BYTE bData;

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_YEAR);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pwYear = RTC_BCDTOBINARY(bData) + 2000;

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_MONTH);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbMonth = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_PORT_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	bData = kInPortByte(RTC_PORT_CMOSDATA);
	*pbDayOfWeek = RTC_BCDTOBINARY(bData);
}
char* kConvertDayOfWeekToString(BYTE bDayOfWeek){
	static char* vpcDayOfWeekString[8] = {	"Error", "Sunday","Monday","Tuesday",\
											"Wednesday","Thursday","Friday","Saturday"};
	if(bDayOfWeek>=8)
		return vpcDayOfWeekString[0];
	return vpcDayOfWeekString[bDayOfWeek];
}
