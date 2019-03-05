/*
 * HardDisk.h
 *
 *  Created on: 2019. 3. 5.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_HARDDISK_H_
#define __02_KERNEL64_SOURCE_HARDDISK_H_
#include "Types.h"
#include "Synchronization.h"


#define HDD_PORT_PRIMARYBASE			0x1F0
#define HDD_PORT_SECONDARYBASE			0x170
/* I/O PORT ADDRESS */
#define HDD_PORT_INDEX_DATA				0x0 //데이터를 읽거나 쓰는 포트 2바이트 단위로 읽는다.
#define HDD_PORT_INDEX_ERROR			0x1 //커맨드 수행 중 발생한 에러를 저장하는 레지스터
#define HDD_PORT_INDEX_SECTORCOUNT		0x2 //읽거나 쓸 섹터 수를 저장하는 레지스터
#define HDD_PORT_INDEX_SECTORNUMBER		0x3 //접근할 섹터 번호를 저장하는 레지스터
#define HDD_PORT_INDEX_CYLINDERLSB		0x4 //접근할 실린더 번호를 저장(하위 8비트)
#define HDD_PORT_INDEX_CYLINDERMSB		0x5 //접근할 실린더 번호를 저장(상위 8비트)
#define HDD_PORT_INDEX_DRIVEHEAD		0x6 //접근할 드라이브 번호와 헤드를 저장
#define HDD_PORT_INDEX_STATUS			0x7 //하드 디스크의 상태를 나타냄(읽기), 하드 디스크로 송신할 커맨드를 저장(쓰기)
#define HDD_PORT_INDEX_COMMAND			0x7
#define HDD_PORT_INDEX_DIGITALOUTPUT	0x206 //인터럽트를 활성화하거나, 디스크를 리셋하는 레지스터(쓰기)
#define HDD_PORT_INDEX_DRIVEADDRESS		0x207 //마스터 또는 슬레이브 하드 디스크 번호와 접근할 헤드 번호를 저장
/* HARD DISK COMMAND */
#define HDD_COMMAND_READ				0x20
#define HDD_COMMAND_WRITE				0x30
#define HDD_COMMAND_INDENTIFY			0xEC
/* STATUS */
#define HDD_STATUS_ERROR				0x01
#define HDD_STATUS_INDEX				0x02
#define HDD_STATUS_CORRECTEDDATA		0x04
#define HDD_STATUS_DATAREQUEST			0x08
#define HDD_STATUS_SEEKCOMPLETE			0x10
#define HDD_STATUS_WRITEFAULT			0x20
#define HDD_STATUS_READY				0x40
#define HDD_STATUS_BUSY					0x80
/* DRIVE/HEAD REGISTER MACRO */
#define HDD_DRIVEANDHEAD_LBA			0xE0 //LBA MODE FLAG
#define HDD_DRIVEANDHEAD_SLAVE			0X10

#define HDD_DIGITALOUTPUT_RESET				0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT	0x01

#define HDD_WAITTIME						500
#define HDD_MAXBULKSECTORCOUNT				256
/* 1 Sector 크기에 맞춤 */
#pragma pack(push,1)
typedef struct kHDDInformationStruct{
	WORD sConfiguration;

	WORD wNumberOfCylinder;
	WORD wReserved1;

	WORD wNumberOfHead;
	WORD wUnformattedBytesPerTrack;
	WORD wUnformattedBytesPerSector;

	WORD wNumberOfSectorPerCylinder;
	WORD wInterSectorGap;
	WORD wBytesInPhaseLock;
	WORD wNumberOfVendorUniqueStatusWord;

	WORD vwSerialNumber[10];
	WORD wControllerType;
	WORD wBufferSize;
	WORD wNumberOfECCBytes;
	WORD vwFirmwareRevision[4];

	WORD vwModelNumber[20];
	WORD vwReserved2[13];

	DWORD dwTotalSector;
	WORD vwReserved3[196];
}HDDINFORMATION;
#pragma pack(pop)
typedef struct kHDDManagerStruct{
	BOOL bHDDDetected;
	BOOL bCanWrite;

	volatile BOOL bPrimaryInterruptOccur;
	volatile BOOL bSecondaryInterruptOccur;
	MUTEX stMutex;

	HDDINFORMATION stHDDInformation;
}HDDMANAGER;
BOOL kInitializeHDD(void);
BOOL kReadHDDInformation(BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation);
int kReadHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
int kWriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
void kSetHDDInterruptFlag(BOOL bPrimary, BOOL bFlag);

static void kSwapByteInWord(WORD* pwData, int iWordCount);
static BYTE kReadHDDStatus(BOOL bPrimary);
static BOOL kIsHDDBusy(BOOL bPrimary);
static BOOL kIsHDDReady(BOOL bPrimary);
static BOOL kWaitForHDDNoBusy(BOOL bPrimary);
static BOOL kWaitForHDDReady(BOOL bPrimary);
static BOOL kWaitForHDDInterrupt(BOOL bPrimary);

#endif /* __02_KERNEL64_SOURCE_HARDDISK_H_ */
