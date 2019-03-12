/*
 * MPConfigurationTable.h
 *
 *  Created on: 2019. 3. 11.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_MPCONFIGURATIONTABLE_H_
#define __02_KERNEL64_SOURCE_MPCONFIGURATIONTABLE_H_
#include "Types.h"

/* MP 플로팅 포인터 특성 바이트 매크로 */
#define MP_FLOATINGPOINTER_FEATUREBYTE1_USEMPTABLE			0x00 /* MP Table 있는지 알려주는 플래그 */
#define MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE				0x80 /* IMCR 레지스터 지원 여부 */
/* 엔트리 타입 */
#define MP_ENTRYTYPE_PROCESSOR								0
#define MP_ENTRYTYPE_BUS									1
#define MP_ENTRYTYPE_IOAPIC									2
#define MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT					3
#define MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT				4
/* 프로세서 cpu 플래그 */
#define MP_PROCESSOR_CPUFLAGS_ENABLE						0x01
#define MP_PROCESSOR_CPUFLAGS_BSP							0x02
/* 버스 타입 스트링 */
#define MP_BUS_TYPESTRING_ISA								"ISA"
#define MP_BUS_TYPESTRING_PCI								"PCI"
#define MP_BUS_TYPESTRING_PCMCIA							"PCMCIA"
#define MP_BUS_TYPESTRING_VESALOCALBUS						"VL"
/* 인터럽트 플래그 */
#define MP_INTERRUPT_FLAGS_CONFORMPOLARITY					0x00
#define MP_INTERRUPT_FLAGS_ACTIVEHIGH						0x01
#define MP_INTERRUPT_FLAGS_ACTIVELOW						0x03
#define MP_INTERRUPT_FLAGS_CONFORMTRIGGER					0x00
#define MP_INTERRUPT_FLAGS_EDGETRIGGER						0x04
#define MP_INTERRUPT_FLAGS_LEVELTRIGGERED					0x0C

#pragma pack(push, 1)
typedef struct kMPFloatingPointerStruct{
	/* 시그니처 */
	char vcString[4];
	/* 테이블 어드레스 */
	DWORD dwMPConfigurationTableAddress;
	/* 사이즈 1로 고정됨 */
	BYTE bLength;
	/* 버전 */
	BYTE bRevision;
	BYTE bCheckSum;
	/* 특성바이트 */
	BYTE vbMPFeatureByte[5];

}MPFLOATINGPOINTER;
typedef struct kMPConfigurationTableHeaderStruct{
	char vcString[4];
	WORD wBaseTableLength;
	BYTE bRevision;
	BYTE bCheckSum;
	char vcOEMIDString[8];
	char vcProductIDString[12];
	DWORD dwOEMTablePointerAddress;
	WORD wOEMTableSize;
	WORD wEntryCount;
	DWORD dwMemoryMapIOAddressOfLocalAPIC;
	WORD wExtendedTableLength;
	BYTE bExtendedTableCheckSum;
	BYTE bReserved;
}MPCONFIGURATIONTABLEHEADER;

typedef struct kProcessorEntryStruct{
	BYTE bEntryType;
	BYTE bLocalAPICID;
	BYTE bLocalAPICVersion;
	BYTE bCPUFlags;
	BYTE vbCPUSignature[4];
	DWORD dwFeatureFlags;
	DWORD dwReserved[2];
}PROCESSORENTRY;
typedef struct kBusEntry{
	BYTE bEntryType;
	BYTE bBusID;
	char vcBusTypeString[6];
}BUSENTRY;

typedef struct kIOAPICEntryStruct{
	BYTE bEntryType;
	BYTE bIOAPICID;
	BYTE bIOAPICVersion;
	BYTE bIOAPICFlags;
	DWORD dwMemoryMapAddress;
}IOAPICENTRY;

typedef struct kIOInterruptAssignmentEntryStruct{
	BYTE bEntryType;
	BYTE bInterruptType;
	WORD wInterruptFlags;
	BYTE bSourceBUSID;
	BYTE bSourceBUSIRQ;
	BYTE bDestinationIOAPICID;
	BYTE bDestinationIOAPICLINTIN;
}IOINTERRUPTASSIGNMENTENTRY;

typedef struct kLocalInterruptEntryStruct{
	BYTE bEntryType;
	BYTE bInterruptType;
	WORD wInterruptFlags;
	BYTE bSourceBUSID;
	BYTE bSourceBUSIRQ;
	BYTE bDestinationLocalAPICID;
	BYTE bDestinationLocalAPICLINTIN;
}LOCALINTERRUPTASSIGNMENTENTRY;
#pragma pack(pop)

typedef struct kMPConfigurationManagerStruct{
	MPFLOATINGPOINTER* pstMPFloatingPointer;
	MPCONFIGURATIONTABLEHEADER* pstMPConfigurationTableHeader;
	QWORD qwBaseEntryStartAddress;
	int iProcessorCount;
	BOOL bUsePICMode;
	BYTE bISABusID;
}MPCONFIGURATIONMANAGER;
BOOL kFindMPFloatingPointerAddress(QWORD* pstAddress);
BOOL kAnalysisMPConfigurationTable(void);
MPCONFIGURATIONMANAGER* kGetMPConfigurationManager(void);
void kPrintMPConfigurationTable( void );
int kGetProcessorCount(void);

static MPCONFIGURATIONMANAGER gs_stMPConfigurationManger;

#endif /* __02_KERNEL64_SOURCE_MPCONFIGURATIONTABLE_H_ */
