/*
 * MultiProcessor.c
 *
 *  Created on: 2019. 3. 19.
 *      Author: DEWH
 */

#include "MultiProcessor.h"
#include "MPConfigurationTable.h"
#include "AssemblyUtility.h"
#include "LocalAPIC.h"
#include "Pit.h"

volatile int g_iWakeUpApplicationProcessorCount = 0;
volatile QWORD g_qwAPICIDAddress = 0;

BOOL kStartUpApplicationProcessor( void )
{
	if(kAnalysisMPConfigurationTable() == FALSE){
		return FALSE;
	}
	kEnableGlobalLocalAPIC();

	kEnableSoftwareLocalAPIC();

	if(kWakeUpApplicationProcessor() == FALSE){
		return FALSE;
	}
	return TRUE;
}
BYTE kGetAPICID( void )
{
	MPCONFIGURATIONTABLEHEADER* pstMPHeader;
	QWORD qwLocalAPICBaseAddress;

	if(g_qwAPICIDAddress == 0){
		pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
		if(pstMPHeader == NULL)
			return 0;
		qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
		g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
	}
	return *((DWORD*)g_qwAPICIDAddress) >> 24;
}
static BOOL kWakeUpApplicationProcessor( void )
{
	MPCONFIGURATIONMANAGER* pstMPMananger;
	MPCONFIGURATIONTABLEHEADER* pstMPHeader;
	QWORD qwLocalAPIBaseAddress;
	BOOL bInterruptFlag;
	int i;

	bInterruptFlag = kSetInterruptFlag(FALSE);

	pstMPMananger = kGetMPConfigurationManager();
	pstMPHeader = pstMPMananger->pstMPConfigurationTableHeader;
	qwLocalAPIBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;

	g_qwAPICIDAddress = qwLocalAPIBaseAddress + APIC_REGISTER_APICID;

	*(DWORD*)(qwLocalAPIBaseAddress + APIC_REGISTER_ICR_LOWER) = APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |\
			APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERY_INIT;

	kWaitUsingDirectPIT(MSTOCOUNT(10));

	if(*(DWORD*)(qwLocalAPIBaseAddress + APIC_REGISTER_ICR_LOWER) & APIC_DELIVERYSTATUS_PENDING){
		kInitializePIT(MSTOCOUNT(1), TRUE);
		kSetInterruptFlag(bInterruptFlag);
		return FALSE;
	}
	for(i = 0; i < 2; i++ ){
		*(DWORD*)(qwLocalAPIBaseAddress + APIC_REGISTER_ICR_LOWER) = APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |\
				APIC_DELIVERY_STARTUP | 0x10;
		kWaitUsingDirectPIT(USTOCOUNT(200));

		if(*(DWORD*)(qwLocalAPIBaseAddress + APIC_REGISTER_ICR_LOWER) & APIC_DELIVERYSTATUS_PENDING){
			kInitializePIT(MSTOCOUNT(1), TRUE);
			kSetInterruptFlag(bInterruptFlag);
			return FALSE;
		}
	}
	kInitializePIT(MSTOCOUNT(1), TRUE);
	kSetInterruptFlag(bInterruptFlag);

	while(g_iWakeUpApplicationProcessorCount < (pstMPMananger->iProcessorCount - 1))
		kSleep(50);
	return TRUE;
}

