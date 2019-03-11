/*
 * MPConfigurationTable.c
 *
 *  Created on: 2019. 3. 11.
 *      Author: DEWH
 */
#include "MPConfigurationTable.h"
static MPCONFIGURATIONMANAGER gs_stMPConfigurationManger;
BOOL kFindMPFloatingPointerAddress(QWORD* pstAddress)
{
	QWORD qwEBDAddress;
	QWORD qwSystemBaseMemory;
	char *pcMPFloatingPointer;
	/* 어드레스는 확장 BIOS 데이터 영역이나, 시스템 기본 메모리의 끝 부분, 또는 BIOS 롬 영역 중 어느 한 곳에 위치한다.  */
	kPrintf("Extended BIOS Data Area = [%X] \n", (DWORD)(*(WORD*)0x040E) * 16);
	kPrintf("System Base Address = [%X] \n", (DWORD)(*(WORD*)0x0413) * 1024);
	/* 확장 BIOS 영역 조사 */
	qwEBDAddress = (QWORD)(*(WORD*)0x040E) * 16;
	for(pcMPFloatingPointer = (char*)qwEBDAddress; pcMPFloatingPointer <=(char*)(qwEBDAddress + 1024);\
	pcMPFloatingPointer++){
		if(kMemCmp(pcMPFloatingPointer, "_MP_", 4)==0){
			kPrintf("MP Floating Pointer Is In EBDA, [0x%X] Address\n",(QWORD)pcMPFloatingPointer);
			*pstAddress = (QWORD)pcMPFloatingPointer;
			return TRUE;
		}
	}
	/* 기본 메로리 영역 조사 */
	qwSystemBaseMemory = (QWORD)(*(WORD*)0x0413) * 1024;
	for(pcMPFloatingPointer = (char*)(qwSystemBaseMemory - 1024); pcMPFloatingPointer <=(char*)qwSystemBaseMemory;\
	pcMPFloatingPointer++){
		if(kMemCmp(pcMPFloatingPointer, "_MP_", 4)==0){
			kPrintf("MP Floating Pointer Is In System Base Memory, [0x%X] Address\n",(QWORD)pcMPFloatingPointer);
			*pstAddress = (QWORD)pcMPFloatingPointer;
			return TRUE;
		}
	}
	/* 바이오스 롬 영역 조사 */
	for(pcMPFloatingPointer = (char*)0x0F0000; pcMPFloatingPointer < (char*)0x0FFFFF;	pcMPFloatingPointer++){
		if(kMemCmp(pcMPFloatingPointer, "_MP_", 4)==0){
			kPrintf("MP Floating Pointer Is In System Base Memory, [0x%X] Address\n",(QWORD)pcMPFloatingPointer);
			*pstAddress = (QWORD)pcMPFloatingPointer;
			return TRUE;
		}
	}
	return FALSE;
}
BOOL kAnalysisMPConfigurationTable(void)
{
	QWORD qwMPFloatingPointerAddress;
	QWORD qwEntryAddress;
	WORD i;
	MPFLOATINGPOINTER* pstMPFloatingPointer;
	MPCONFIGURATIONTABLEHEADER* pstMPConfigurationHeader;
	PROCESSORENTRY* pstProcessorEntry;
	BUSENTRY* pstBustEntry;
	BYTE bEntryType;
	/* MPCONFIGURATIONMANAGER을 세팅하는 함수 */
	kMemSet(&gs_stMPConfigurationManger, 0, sizeof(MPCONFIGURATIONMANAGER));
	if(kFindMPFloatingPointerAddress(&qwMPFloatingPointerAddress)==FALSE)
		return FALSE;
	pstMPFloatingPointer = (MPFLOATINGPOINTER*)qwMPFloatingPointerAddress;
	gs_stMPConfigurationManger.pstMPFloatingPointer = pstMPFloatingPointer;
	pstMPConfigurationHeader = (MPCONFIGURATIONTABLEHEADER*) ((QWORD)((pstMPFloatingPointer->dwMPConfigurationTableAddress)&0xFFFFFFFF));
	if(pstMPFloatingPointer->vbMPFeatureByte[1] & MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE)
		gs_stMPConfigurationManger.bUsePICMode = TRUE;
	gs_stMPConfigurationManger.pstMPConfigurationTableHeader = pstMPConfigurationHeader;
	gs_stMPConfigurationManger.qwBaseEntryStartAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress + \
			sizeof(MPCONFIGURATIONTABLEHEADER);

	qwEntryAddress = gs_stMPConfigurationManger.qwBaseEntryStartAddress;
	for(i = 0; i < pstMPConfigurationHeader->wEntryCount; i++ ){
		bEntryType = *(BYTE*)qwEntryAddress;
		switch(bEntryType){
		case MP_ENTRYTYPE_PROCESSOR:
			pstProcessorEntry = (PROCESSORENTRY*)qwEntryAddress;
			if(pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE)
				gs_stMPConfigurationManger.iProcessorCount++;
			qwEntryAddress += sizeof(PROCESSORENTRY);
			break;
		case MP_ENTRYTYPE_BUS:
			pstBustEntry = (BUSENTRY*)qwEntryAddress;
			if(kMemCmp(pstBustEntry->vcBusTypeString, MP_BUS_TYPESTRING_ISA, kStrlen(MP_BUS_TYPESTRING_ISA))==0)
				gs_stMPConfigurationManger.bISABusID = pstBustEntry->bBusID;
			qwEntryAddress += sizeof(BUSENTRY);
			break;
		case MP_ENTRYTYPE_IOAPIC:
		case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
		case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
		default:
			qwEntryAddress += 8;
			break;
		}
	}
	return TRUE;

}
MPCONFIGURATIONMANAGER* kGetMPConfigurationManager(void)
{
	return &gs_stMPConfigurationManger;
}
void kPrintMPConfigurationTable( void )
{
    MPCONFIGURATIONMANAGER* pstMPConfigurationManager;
    QWORD qwMPFloatingPointerAddress;
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    MPCONFIGURATIONTABLEHEADER* pstMPTableHeader;
    PROCESSORENTRY* pstProcessorEntry;
    BUSENTRY* pstBusEntry;
    IOAPICENTRY* pstIOAPICEntry;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    LOCALINTERRUPTASSIGNMENTENTRY* pstLocalAssignmentEntry;
    QWORD qwBaseEntryAddress;
    char vcStringBuffer[ 20 ];
    WORD i;
    BYTE bEntryType;
    // 화면에 출력할 문자열
    char* vpcInterruptType[ 4 ] = { "INT", "NMI", "SMI", "ExtINT" };
    char* vpcInterruptFlagsPO[ 4 ] = { "Conform", "Active High",
            "Reserved", "Active Low" };
    char* vpcInterruptFlagsEL[ 4 ] = { "Conform", "Edge-Trigger", "Reserved",
            "Level-Trigger"};

    //==========================================================================
    // MP 설정 테이블 처리 함수를 먼저 호출하여 시스템 처리에 필요한 정보를 저장
    //==========================================================================
    kPrintf( "================ MP Configuration Table Summary ================\n" );
    pstMPConfigurationManager = kGetMPConfigurationManager();
    if( ( pstMPConfigurationManager->qwBaseEntryStartAddress == 0 ) &&
        ( kAnalysisMPConfigurationTable() == FALSE ) )
    {
        kPrintf( "MP Configuration Table Analysis Fail\n" );
        return ;
    }
    kPrintf( "MP Configuration Table Analysis Success\n" );

    kPrintf( "MP Floating Pointer Address : 0x%Q\n",
            pstMPConfigurationManager->pstMPFloatingPointer );
    kPrintf( "PIC Mode Support : %d\n", pstMPConfigurationManager->bUsePICMode );
    kPrintf( "MP Configuration Table Header Address : 0x%Q\n",
            pstMPConfigurationManager->pstMPConfigurationTableHeader );
    kPrintf( "Base MP Configuration Table Entry Start Address : 0x%Q\n",
            pstMPConfigurationManager->qwBaseEntryStartAddress );
    kPrintf( "Processor Count : %d\n", pstMPConfigurationManager->iProcessorCount );
    kPrintf( "ISA Bus ID : %d\n", pstMPConfigurationManager->bISABusID );

    kPrintf( "Press any key to continue... ('q' is exit) : " );
    if( kGetCh() == 'q' )
    {
        kPrintf( "\n" );
        return ;
    }
    kPrintf( "\n" );

    //==========================================================================
    // MP 플로팅 포인터 정보를 출력
    //==========================================================================
    kPrintf( "=================== MP Floating Pointer ===================\n" );
    pstMPFloatingPointer = pstMPConfigurationManager->pstMPFloatingPointer;
    kMemCpy( vcStringBuffer, pstMPFloatingPointer->vcString, 4 );
    vcStringBuffer[ 4 ] = '\0';
    kPrintf( "Signature : %s\n", vcStringBuffer );
    kPrintf( "MP Configuration Table Address : 0x%Q\n",
            pstMPFloatingPointer->dwMPConfigurationTableAddress );
    kPrintf( "Length : %d * 16 Byte\n", pstMPFloatingPointer->bLength );
    kPrintf( "Version : %d\n", pstMPFloatingPointer->bRevision );
    kPrintf( "CheckSum : 0x%X\n", pstMPFloatingPointer->bCheckSum );
    kPrintf( "Feature Byte 1 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 0 ] );
    // MP 설정 테이블 사용 여부 출력
    if( pstMPFloatingPointer->vbMPFeatureByte[ 0 ] == 0 )
    {
        kPrintf( "(Use MP Configuration Table)\n" );
    }
    else
    {
        kPrintf( "(Use Default Configuration)\n" );
    }
    // PIC 모드 지원 여부 출력
    kPrintf( "Feature Byte 2 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 1 ] );
    if( pstMPFloatingPointer->vbMPFeatureByte[ 2 ] &
            MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE )
    {
        kPrintf( "(PIC Mode Support)\n" );
    }
    else
    {
        kPrintf( "(Virtual Wire Mode Support)\n" );
    }

    //==========================================================================
    // MP 설정 테이블 헤더 정보를 출력
    //==========================================================================
    kPrintf( "\n=============== MP Configuration Table Header ===============\n" );
    pstMPTableHeader = pstMPConfigurationManager->pstMPConfigurationTableHeader;
    kMemCpy( vcStringBuffer, pstMPTableHeader->vcString, 4 );
    vcStringBuffer[ 4 ] = '\0';
    kPrintf( "Signature : %s\n", vcStringBuffer );
    kPrintf( "Length : %d Byte\n", pstMPTableHeader->wBaseTableLength );
    kPrintf( "Version : %d\n", pstMPTableHeader->bRevision );
    kPrintf( "CheckSum : 0x%X\n", pstMPTableHeader->bCheckSum );
    kMemCpy( vcStringBuffer, pstMPTableHeader->vcOEMIDString, 8 );
    vcStringBuffer[ 8 ] = '\0';
    kPrintf( "OEM ID String : %s\n", vcStringBuffer );
    kMemCpy( vcStringBuffer, pstMPTableHeader->vcProductIDString, 12 );
    vcStringBuffer[ 12 ] = '\0';
    kPrintf( "Product ID String : %s\n", vcStringBuffer );
    kPrintf( "OEM Table Pointer : 0x%X\n",
             pstMPTableHeader->dwOEMTablePointerAddress );
    kPrintf( "OEM Table Size : %d Byte\n", pstMPTableHeader->wOEMTableSize );
    kPrintf( "Entry Count : %d\n", pstMPTableHeader->wEntryCount );
    kPrintf( "Memory Mapped I/O Address Of Local APIC : 0x%X\n",
            pstMPTableHeader->dwMemoryMapIOAddressOfLocalAPIC );
    kPrintf( "Extended Table Length : %d Byte\n",
            pstMPTableHeader->wExtendedTableLength );
    kPrintf( "Extended Table Checksum : 0x%X\n",
            pstMPTableHeader->bExtendedTableCheckSum );

    kPrintf( "Press any key to continue... ('q' is exit) : " );
    if( kGetCh() == 'q' )
    {
        kPrintf( "\n" );
        return ;
    }
    kPrintf( "\n" );

    //==========================================================================
    // 기본 MP 설정 테이블 엔트리 정보를 출력
    //==========================================================================
    kPrintf( "\n============= Base MP Configuration Table Entry =============\n" );
    qwBaseEntryAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress +
        sizeof( MPCONFIGURATIONTABLEHEADER );
    for( i = 0 ; i < pstMPTableHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwBaseEntryAddress;
        switch( bEntryType )
        {
            // 프로세스 엔트리 정보 출력
        case MP_ENTRYTYPE_PROCESSOR:
            pstProcessorEntry = ( PROCESSORENTRY* ) qwBaseEntryAddress;
            kPrintf( "Entry Type : Processor\n" );
            kPrintf( "Local APIC ID : %d\n", pstProcessorEntry->bLocalAPICID );
            kPrintf( "Local APIC Version : 0x%X\n", pstProcessorEntry->bLocalAPICVersion );
            kPrintf( "CPU Flags : 0x%X ", pstProcessorEntry->bCPUFlags );
            // Enable/Disable 출력
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE )
            {
                kPrintf( "(Enable, " );
            }
            else
            {
                kPrintf( "(Disable, " );
            }
            // BSP/AP 출력
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_BSP )
            {
                kPrintf( "BSP)\n" );
            }
            else
            {
                kPrintf( "AP)\n" );
            }
            kPrintf( "CPU Signature : 0x%X\n", pstProcessorEntry->vbCPUSignature );
            kPrintf( "Feature Flags : 0x%X\n\n", pstProcessorEntry->dwFeatureFlags );

            // 프로세스 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
            qwBaseEntryAddress += sizeof( PROCESSORENTRY );
            break;

            // 버스 엔트리 정보 출력
        case MP_ENTRYTYPE_BUS:
            pstBusEntry = ( BUSENTRY* ) qwBaseEntryAddress;
            kPrintf( "Entry Type : Bus\n" );
            kPrintf( "Bus ID : %d\n", pstBusEntry->bBusID );
            kMemCpy( vcStringBuffer, pstBusEntry->vcBusTypeString, 6 );
            vcStringBuffer[ 6 ] = '\0';
            kPrintf( "Bus Type String : %s\n\n", vcStringBuffer );

            // 버스 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
            qwBaseEntryAddress += sizeof( BUSENTRY );
            break;

            // I/O APIC 엔트리
        case MP_ENTRYTYPE_IOAPIC:
            pstIOAPICEntry = ( IOAPICENTRY* ) qwBaseEntryAddress;
            kPrintf( "Entry Type : I/O APIC\n" );
            kPrintf( "I/O APIC ID : %d\n", pstIOAPICEntry->bIOAPICID );
            kPrintf( "I/O APIC Version : 0x%X\n", pstIOAPICEntry->bIOAPICVersion );
            kPrintf( "I/O APIC Flags : 0x%X ", pstIOAPICEntry->bIOAPICFlags );
            // Enable/Disable 출력
            if( pstIOAPICEntry->bIOAPICFlags == 1 )
            {
                kPrintf( "(Enable)\n" );
            }
            else
            {
                kPrintf( "(Disable)\n" );
            }
            kPrintf( "Memory Mapped I/O Address : 0x%X\n\n",
                    pstIOAPICEntry->dwMemoryMapAddress );

            // I/O APIC 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
            qwBaseEntryAddress += sizeof( IOAPICENTRY );
            break;

            // I/O 인터럽트 지정 엔트리
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* )
                qwBaseEntryAddress;
            kPrintf( "Entry Type : I/O Interrupt Assignment\n" );
            kPrintf( "Interrupt Type : 0x%X ", pstIOAssignmentEntry->bInterruptType );
            // 인터럽트 타입 출력
            kPrintf( "(%s)\n", vpcInterruptType[ pstIOAssignmentEntry->bInterruptType ] );
            kPrintf( "I/O Interrupt Flags : 0x%X ", pstIOAssignmentEntry->wInterruptFlags );
            // 극성과 트리거 모드 출력
            kPrintf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstIOAssignmentEntry->wInterruptFlags & 0x03 ],
                    vpcInterruptFlagsEL[ ( pstIOAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );
            kPrintf( "Source BUS ID : %d\n", pstIOAssignmentEntry->bSourceBUSID );
            kPrintf( "Source BUS IRQ : %d\n", pstIOAssignmentEntry->bSourceBUSIRQ );
            kPrintf( "Destination I/O APIC ID : %d\n",
                     pstIOAssignmentEntry->bDestinationIOAPICID );
            kPrintf( "Destination I/O APIC INTIN : %d\n\n",
                     pstIOAssignmentEntry->bDestinationIOAPICLINTIN );

            // I/O 인터럽트 지정 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
            qwBaseEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;

            // 로컬 인터럽트 지정 엔트리
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            pstLocalAssignmentEntry = ( LOCALINTERRUPTASSIGNMENTENTRY* )
                qwBaseEntryAddress;
            kPrintf( "Entry Type : Local Interrupt Assignment\n" );
            kPrintf( "Interrupt Type : 0x%X ", pstLocalAssignmentEntry->bInterruptType );
            // 인터럽트 타입 출력
            kPrintf( "(%s)\n", vpcInterruptType[ pstLocalAssignmentEntry->bInterruptType ] );
            kPrintf( "I/O Interrupt Flags : 0x%X ", pstLocalAssignmentEntry->wInterruptFlags );
            // 극성과 트리거 모드 출력
            kPrintf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstLocalAssignmentEntry->wInterruptFlags & 0x03 ],
                    vpcInterruptFlagsEL[ ( pstLocalAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );
            kPrintf( "Source BUS ID : %d\n", pstLocalAssignmentEntry->bSourceBUSID );
            kPrintf( "Source BUS IRQ : %d\n", pstLocalAssignmentEntry->bSourceBUSIRQ );
            kPrintf( "Destination Local APIC ID : %d\n",
                     pstLocalAssignmentEntry->bDestinationLocalAPICID );
            kPrintf( "Destination Local APIC LINTIN : %d\n\n",
                     pstLocalAssignmentEntry->bDestinationLocalAPICLINTIN );

            // 로컬 인터럽트 지정 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
            qwBaseEntryAddress += sizeof( LOCALINTERRUPTASSIGNMENTENTRY );
            break;

        default :
            kPrintf( "Unknown Entry Type. %d\n", bEntryType );
            break;
        }

        // 3개를 출력하고 나면 키 입력을 대기
        if( ( i != 0 ) && ( ( ( i + 1 ) % 3 ) == 0 ) )
        {
            kPrintf( "Press any key to continue... ('q' is exit) : " );
            if( kGetCh() == 'q' )
            {
                kPrintf( "\n" );
                return ;
            }
            kPrintf( "\n" );
        }
    }
}
int kGetProcessorCount(void)
{
	return gs_stMPConfigurationManger.iProcessorCount;
}

