/*
 * FileSystem.h
 *
 *  Created on: 2019. 3. 6.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_FILESYSTEM_H_
#define __02_KERNEL64_SOURCE_FILESYSTEM_H_
#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"
//MINT FILESYSTEM SIGNATURE
#define FILESYSTEM_SIGNATURE			0x7E38CF10
/* 1 CLUSTER == 8 SECTOR */
#define FILESYSTEM_SECTORSPERCLUSTER	8
/* FILESYSTEM LAST CLUSTER */
/* CLUSTER INDEX SHOULD BE LESS THAN 4BYTE */
#define FILESYSTEM_LASTCLUSTER			0xFFFFFFFF
#define FILESYSTEM_FREECLUSTER			0x00
/* ROOT DIRECTORY CAN HAVE LIMITED NUMBER OF FILE */
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT	((FILESYSTEM_SECTORSPERCLUSTER * 512)/ sizeof(DIRECTORYENTRY))

#define FILESYSTEM_CLUSTERSIZE			( FILESYSTEM_SECTORSPERCLUSTER * 512 )

#define FILESYSTEM_MAXFILENAMELENGTH	24

#define PARTITION_BOOTABLE				0x80
#define PARTITION_UNBOOTABLE			0x00
/* 외부 저장 장치 제어 함수 포인터 타입 정의 */
typedef BOOL (* fReadHDDInformation) (BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation);
typedef int	(*fReadHDDSector) (BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
typedef int (*fWriteHDDSector) (BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
/* struct */
#pragma pack(push, 1)
typedef struct kPartitionStruct{
	/* */
	BYTE bBootableFlag;
	/* Deprecated */
	BYTE vbStartingCHSAddress[3];
	BYTE bPartitionType;
	/* Deprecated */
	BYTE vbEndingCHSADDRESS[3];
	DWORD dwStartingLBAAddress;
	DWORD dwSizeInSector;
}PARTITION;

typedef struct kMBRStruct{
	BYTE vbBootCode [ 430 ];
	DWORD dwSignature;
	DWORD dwReservedSectorCount;
	DWORD dwClusterLinkSectorCount;
	DWORD dwTotalClusterCount;
	PARTITION vstPartition[4];
	BYTE vbBootLoaderSignature[ 2 ];
}MBR;
typedef struct kDirectoryEntryStruct{
	char vcFileName[FILESYSTEM_MAXFILENAMELENGTH];
	DWORD dwFileSize;
	DWORD dwStartClusterIndex;
}DIRECTORYENTRY;

typedef struct kFileSystemManagerStruct{
	BOOL bMounted;
	DWORD dwReservedSectorCount;
	DWORD dwClusterLinkAreaStartAddress;
	DWORD dwClusterLinkAreaSize;
	DWORD dwDataAreaStartAddress;
	DWORD dwTotalClusterCount;

	DWORD dwLastAllocatedClusterLinkSectorOffset;

	MUTEX stMutex;
}FILESYSTEMMANAGER;
#pragma pack(pop)
BOOL kInitializeFileSystem(void);
BOOL kFormat( void );
BOOL kMount( void );
BOOL kGetHDDInformation(HDDINFORMATION* pstInformation);
/* 클러스터 링크 테이블 제어 함수 */
BOOL kReadClusterLinkTable(DWORD dwOffset, BYTE* pbBuffer);
BOOL kWriteClusterLinkTable(DWORD dwOffset, BYTE* pbBuffer);
/* 클러스터 제어 함수 */
BOOL kReadCluster(DWORD dwOffset, BYTE* pbBuffer);
BOOL kWriteCluster(DWORD dwOffset, BYTE* pbBuffer);

DWORD kFindFreeCluster( void );
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData);
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData);
int kFindFreeDirectoryEntry( void );
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry);
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry);
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry);
void kGetFileSystemInformation(FILESYSTEMMANAGER* pstManager);

#endif /* __02_KERNEL64_SOURCE_FILESYSTEM_H_ */
