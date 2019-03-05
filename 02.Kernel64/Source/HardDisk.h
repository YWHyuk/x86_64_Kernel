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
#define HDD_PORT_INDEX_DATA				0x0 //�����͸� �аų� ���� ��Ʈ 2����Ʈ ������ �д´�.
#define HDD_PORT_INDEX_ERROR			0x1 //Ŀ�ǵ� ���� �� �߻��� ������ �����ϴ� ��������
#define HDD_PORT_INDEX_SECTORCOUNT		0x2 //�аų� �� ���� ���� �����ϴ� ��������
#define HDD_PORT_INDEX_SECTORNUMBER		0x3 //������ ���� ��ȣ�� �����ϴ� ��������
#define HDD_PORT_INDEX_CYLINDERLSB		0x4 //������ �Ǹ��� ��ȣ�� ����(���� 8��Ʈ)
#define HDD_PORT_INDEX_CYLINDERMSB		0x5 //������ �Ǹ��� ��ȣ�� ����(���� 8��Ʈ)
#define HDD_PORT_INDEX_DRIVEHEAD		0x6 //������ ����̺� ��ȣ�� ��带 ����
#define HDD_PORT_INDEX_STATUS			0x7 //�ϵ� ��ũ�� ���¸� ��Ÿ��(�б�), �ϵ� ��ũ�� �۽��� Ŀ�ǵ带 ����(����)
#define HDD_PORT_INDEX_COMMAND			0x7
#define HDD_PORT_INDEX_DIGITALOUTPUT	0x206 //���ͷ�Ʈ�� Ȱ��ȭ�ϰų�, ��ũ�� �����ϴ� ��������(����)
#define HDD_PORT_INDEX_DRIVEADDRESS		0x207 //������ �Ǵ� �����̺� �ϵ� ��ũ ��ȣ�� ������ ��� ��ȣ�� ����
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
/* 1 Sector ũ�⿡ ���� */
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
