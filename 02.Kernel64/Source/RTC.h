/*
 * RTC.h
 *
 *  Created on: 2019. 2. 18.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_RTC_H_
#define __02_KERNEL64_SOURCE_RTC_H_
#include "Types.h"

//Port I/O Map Address
#define RTC_PORT_CMOSADDRESS		0x70
#define RTC_PORT_CMOSDATA			0x71
//CMOS DATA Address;
#define RTC_ADDRESS_SECOND			0x00
#define RTC_ADDRESS_MINUTE			0x02
#define RTC_ADDRESS_HOUR			0x04
#define RTC_ADDRESS_DAYOFWEEK		0x06
#define RTC_ADDRESS_DAYOFMONTH		0x07
#define RTC_ADDRESS_MONTH			0x08
#define RTC_ADDRESS_YEAR			0x09

#define RTC_BCDTOBINARY( x ) ( ( ( x ) >> 4 ) * 10 ) + ( ( x ) & 0xF )

void kReadRTCTIME(BYTE* pbHour,BYTE* pbMinute, BYTE* pbSecond);
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek);
char* kConvertDayOfWeekToString(BYTE bDayOfWeek);

#endif /* __02_KERNEL64_SOURCE_RTC_H_ */
