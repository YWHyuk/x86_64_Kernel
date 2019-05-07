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
#include "Task.h"
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
/* Maximum Number of Handle */
#define FILESYSTEM_HANDLE_MAXCOUNT		(TASK_MAXCOUNT * 3)
#define FILESYSTEM_TYPE_FREE			0
#define FILESYSTEM_TYPE_FILE			1
#define FILESYSTEM_TYPE_DIRECTORY		2
/* Seek option */
#define FILESYSTEM_SEEK_SET				0
#define FILESYSTEM_SEEK_CUR				1
#define FILESYSTEM_SEEK_END				2

#define FILESYSTEM_MAXFILENAMELENGTH	24

#define PARTITION_BOOTABLE				0x80
#define PARTITION_UNBOOTABLE			0x00

/* 파일 시스템 함수를 표준 입출력 함수 이름으로 재정의 */
#define fopen		kOpenFile
#define fread		kReadFile
#define fwrite		kWriteFile
#define	fseek		kSeekFile
#define fclose		kCloseFile
#define remove		kRemoveFile
#define opendir		kOpenDirectory
#define readdir		kReadDirectory
#define rewinddir	kRewindDirectory
#define closedir	kCloseDirectory

#define SEEK_SET	FILESYSTEM_SEEK_SET
#define SEEK_CUR	FILESYSTEM_SEEK_CUR
#define SEEK_END	FILESYSTEM_SEEK_END

#define size_T		DWORD
#define dirent		kDirectoryEntryStruct
#define d_name		vcFileName

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
/* 파일 핸들과 디렉토리 핸들 */
typedef struct kFileHandleStruct{
	/* 저장된 디렉토리의 디렉토리 엔트리 인덱스 */
	int iDirectoryEntryoffset;
	/* 파일 사이즈 */
	DWORD dwFileSize;
	/* 시작 클러스터 인덱스 */
	DWORD dwStartClusterIndex;
	/* 현재 클러스터 인덱스 */
	DWORD dwCurrentClusterIndex;
	/* 이전 클러스터의 인덱스 */
	DWORD dwPreviousClusterIndex;
	/* 현재 클러스터의 오프셋 */
	DWORD dwCurrentOffset;
}FILEHANDLE;

typedef struct kDirectoryHandlerStruct{
	/* 루트 디렉토리를 저장해둔 버퍼 */
	DIRECTORYENTRY* pstDirectoryBuffer;
	/* 디렉토리 엔트리 오프셋 */
	int iCurrentOffset;
}DIRECTORYHANDLE;

/* 파일과 디렉토리에 대한 정보가 들어있는 자료구조 */
typedef struct kFileDirectoryHandle{
	/* 파일인지 디렉토리인지 타입 저장 */
	BYTE bType;
	union{
		FILEHANDLE stFileHandle;
		DIRECTORYHANDLE stDirectoryHandle;
	};
}FILE,DIR;

/* 파일 시스템을 관리하는 구조체 */
typedef struct kFileSystemManagerStruct{
	BOOL bMounted;
	DWORD dwReservedSectorCount;
	DWORD dwClusterLinkAreaStartAddress;
	DWORD dwClusterLinkAreaSize;
	DWORD dwDataAreaStartAddress;
	DWORD dwTotalClusterCount;
	DWORD dwLastAllocatedClusterLinkSectorOffset;
	MUTEX stMutex;
	FILE* pstHandlePool;
}FILESYSTEMMANAGER;
#pragma pack(pop)

BOOL kInitializeFileSystem(void);
BOOL kFormat( void );
BOOL kMount( void );
BOOL kGetHDDInformation(HDDINFORMATION* pstInformation);
/*==========================================================================
 * 							Low Level Functions
 * ==========================================================================*/
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
/*==========================================================================
 * 							High Level Functions
 * ==========================================================================*/
FILE* kOpenFile(const char* pcFileName, const char* pcMode);
DWORD kReadFile(void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile);
DWORD kWriteFile(const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile);
int kSeekFile(FILE* pstFile, int iOffset, int iOrigin);
int kCloseFile(FILE* pstFile);
int kRemoveFile(const char* pcFileName);
DIR* kOpenDirectory(const char* pcDirectoryName);
struct kDirectoryEntryStruct* kReadDirectory(DIR* pstDirectory);
void kRewindDirectory(DIR* pstDirectory);
int kCloseDirectory(DIR* pstDirectory);
BOOL kWriteZero(FILE* pstFile, DWORD dwCount);
BOOL kIsFileOpened(const DIRECTORYENTRY* pstEntry);
//static void* kAllocateFileDirectoryHandle(void);
//static void kFreeFileDirectoryHandle(FILE* pstFile);
//static BOOL kCreateFile(const char* pcFileName, DIRECTORYENTRY* pstEntry,int* piDirectoryEntryIndex);
//static BOOL kFreeClusterUntilEnd(DWORD dwClusterIndex);
//static BOOL kUpdateDirectoryEntry(FILEHANDLE* pstFileHandle);
#endif /* __02_KERNEL64_SOURCE_FILESYSTEM_H_ */
