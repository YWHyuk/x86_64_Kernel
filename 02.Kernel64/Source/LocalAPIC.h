/*
 * LocalAPIC.h
 *
 *  Created on: 2019. 3. 12.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_LOCALAPIC_H_
#define __02_KERNEL64_SOURCE_LOCALAPIC_H_
#include "Types.h"

/* 로컬 APIC 레지스터 오프셋 관련 매크로 */
#define APIC_REGISTER_EOI							0x00B0
#define APIC_REGISTER_SVR							0x00F0
#define APIC_REGISTER_APICID						0x0020
#define APIC_REGISTER_TASKPRIORITY					0x0080
#define APIC_REGISTER_TIMER							0x0320
#define APIC_REGISTER_THERMALSENSOR					0x0330
#define APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER	0x0340
#define APIC_REGISTER_LINT0							0x0350
#define APIC_REGISTER_LINT1							0x0360
#define APIC_REGISTER_ERROR							0x0370
#define APIC_REGISTER_ICR_LOWER						0x0300
#define APIC_REGISTER_ICR_UPPER						0x0310

/* 전달 모드 */
#define APIC_DELIVERY_FIXED							0x0000
#define APIC_DELIVERY_LOWESTPRIORITY				0x0100
#define APIC_DELIVERY_SMI							0x0200
#define APIC_DELIVERY_NMI							0x0400
#define APIC_DELIVERY_INIT							0x0500
#define APIC_DELIVERY_STARTUP						0x0600
#define APIC_DELIVERY_EXTINT						0x0700

#define APIC_DESTINATIONMODE_PHYSICAL				0x0000
#define APIC_DESTINATIONMODE_LOGICAL				0x0800

#define APIC_DELIVERYSTATUS_IDEL					0x0000
#define APIC_DELIVERYSTATUS_PENDING					0x1000

#define APIC_LEVEL_DEASSERT							0x0000
#define APIC_LEVEL_ASSERT							0x4000

#define APIC_TRIGGERMODE_EDGE						0x0000
#define APIC_TRIGGERMODE_LEVEL						0x8000

#define APIC_DESTINATIONSHORTHAND_NOSHORTHAND		0x000000
#define APIC_DESTINATIONSHORTHAND_SELF				0x040000
#define APIC_DESTINATIONSHORTHAND_ALLINCLUDINGSELF	0x080000
#define APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF	0x0C0000

QWORD kGetLocalAPICBaseAddress( void );
void kEnableSoftwareLocalAPIC( void );


#endif /* __02_KERNEL64_SOURCE_LOCALAPIC_H_ */
