/*
 * FileSystem.c
 *
 *  Created on: 2019. 3. 6.
 *      Author: DEWH
 */
#include "FileSystem.h"
//하드 디스크 제어에 관련된 함수 포인터 선언
fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;
static FILESYSTEMMANAGER gs_stFileSystemManager;
/* 한 클러스터를 담을 공간 */
static BYTE gs_vbTempBuffer[FILESYSTEM_SECTORSPERCLUSTER * 512];
BOOL kInitializeFileSystem(void)
{
	/*
	 * 연결된 외부 장치를 검색...
	 */
	kMemSet(&(gs_stFileSystemManager), 0, sizeof(FILESYSTEMMANAGER));
	kInitializeMutex(&(gs_stFileSystemManager.stMutex));
	/* 하드 디스크를 검색 존재한다면, 외부 장치 제어 함수를 연결  */
	if(kInitializeHDD()==TRUE){
		gs_pfReadHDDInformation = kReadHDDInformation;
		gs_pfReadHDDSector = kReadHDDSector;
		gs_pfWriteHDDSector= kWriteHDDSector;
	}else{
		return FALSE;
	}
	/* 파일 시스템 연결 */
	if( kMount() == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}
BOOL kFormat( void )
{
	MBR* pstMBR;
	HDDINFORMATION* pstInformation;
	DWORD dwTotalSector;
	DWORD dwTotalCluster ,dwClusterLinkSectorCount;
	DWORD dwTemp;
	int i;
	kLock(&(gs_stFileSystemManager.stMutex));
	/* */
	if(gs_pfReadHDDInformation(TRUE,TRUE,(HDDINFORMATION*)gs_vbTempBuffer)==FALSE)
		goto ERROR;
	pstInformation = (HDDINFORMATION*)gs_vbTempBuffer;
	dwTotalSector = pstInformation->dwTotalSector;
	dwTemp = (dwTotalSector - 1) / 1025;
	if((dwTemp * 1025 + 1) == dwTotalSector){
		dwTotalCluster = 128 * dwTemp;
	}else{
		dwTemp = (dwTotalSector - 2) / 1025;
		dwTotalCluster = (dwTotalSector - 2 -dwTemp) / 8;
	}
	/*
	 * 1 + reserved_sector + ceiling((4*cluster_count)/512) + 8*cluster_count = total_sector
	 * case1: cluster_count = 128n + a; (0<a<128)
	 * 	1 + reserved_sector + n + 1 + 8*cluster_count = total_sector
	 * 	1025n + 8a = total_sector -2
	 * 	n = (total_sector - 2) / 1025
	 * case2: cluster_count = 128n
	 *  1 + reserved_sector + n + 8**cluster_count = total_sector
	 *  1025n = total_sector - 1
	 *  n = (total_sector -1) / 1025
	 *  수식을 통해 cluster_count를 구해보자
	 */
	dwClusterLinkSectorCount = ((dwTotalCluster * 4) + 511) /512;
	if(gs_pfReadHDDSector(TRUE,TRUE,0,1,gs_vbTempBuffer)==FALSE)
		goto ERROR;
	pstMBR = (MBR*)gs_vbTempBuffer;
	pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
	pstMBR->dwReservedSectorCount = 0;
	pstMBR->dwClusterLinkSectorCount =  dwClusterLinkSectorCount;
	pstMBR->dwTotalClusterCount = dwTotalCluster;
	kMemSet(pstMBR->vstPartition, 0, sizeof(pstMBR->vstPartition));

	if(gs_pfWriteHDDSector(TRUE,TRUE,0,1,gs_vbTempBuffer)==FALSE)
		goto ERROR;
	kMemSet(gs_vbTempBuffer, 0, 512);
	kPrintf("#%d\n",dwClusterLinkSectorCount);
	for(i = 0;i < dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER; i++){
		if( i == 0)
			((DWORD*)(gs_vbTempBuffer))[0] = FILESYSTEM_LASTCLUSTER;
		else
			((DWORD*)(gs_vbTempBuffer))[0] = FILESYSTEM_FREECLUSTER;
		if(gs_pfWriteHDDSector(TRUE,TRUE, i + 1, 1, gs_vbTempBuffer)==FALSE)
			goto ERROR;
	}

	kUnlock(&(gs_stFileSystemManager.stMutex));
	return TRUE;
ERROR:
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return FALSE;
}
BOOL kMount( void )
{
	/* filesystemmanger을 실제 데이터와 연결한다. */
	MBR* pstMBR;
	kLock(&(gs_stFileSystemManager.stMutex));
	if(gs_pfReadHDDSector(TRUE,TRUE,0,1,gs_vbTempBuffer)==FALSE)
		goto ERROR;
	pstMBR = (MBR*)gs_vbTempBuffer;
	/* Check the signature */
	if(pstMBR->dwSignature != FILESYSTEM_SIGNATURE)
		goto ERROR;
	gs_stFileSystemManager.bMounted = TRUE;

	gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
	gs_stFileSystemManager.dwClusterLinkAreaStartAddress = pstMBR->dwReservedSectorCount + 1;
	gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
	gs_stFileSystemManager.dwDataAreaStartAddress =
			pstMBR->dwReservedSectorCount + 1 + pstMBR->dwClusterLinkSectorCount;
	gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;
	gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = 0;
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return TRUE;
ERROR:
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return FALSE;
}
BOOL kGetHDDInformation(HDDINFORMATION* pstInformation)
{
	BOOL bReturn;
	kLock(&(gs_stFileSystemManager.stMutex));
	if(gs_pfReadHDDInformation != NULL)
		bReturn = gs_pfReadHDDInformation(TRUE,TRUE,pstInformation);
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return bReturn;
}
/* 클러스터 링크 테이블 제어 함수 */
BOOL kReadClusterLinkTable(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfReadHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwClusterLinkAreaStartAddress + dwOffset,\
				1, pbBuffer);
}
BOOL kWriteClusterLinkTable(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfWriteHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwClusterLinkAreaStartAddress + dwOffset,\
				1, pbBuffer);
}
/* 클러스터 제어 함수 */
BOOL kReadCluster(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfReadHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwDataAreaStartAddress + \
				dwOffset * FILESYSTEM_SECTORSPERCLUSTER, 1, pbBuffer);
}
BOOL kWriteCluster(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfWriteHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwDataAreaStartAddress + \
			dwOffset * FILESYSTEM_SECTORSPERCLUSTER, 1, pbBuffer);
}
DWORD kFindFreeCluster( void )
{
	DWORD* pdwClusterLink;
	DWORD dwTotalClusterCount;
	DWORD dwSectorOffset;
	DWORD dwCurrentSectorOffset;
	DWORD dwClusterOffset;
	int i,j;
	/* 클러스터 링크 읽기 */
	if(gs_stFileSystemManager.bMounted == FALSE)
		return FILESYSTEM_LASTCLUSTER;
	/* 책엔 락이 걸려 있지않은데 왜 안하지?
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */

	kLock(&(gs_stFileSystemManager.stMutex));
	dwSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;
	/* 클러스터 링크가 차지하는 만큼 한섹터를 읽고 반복한다. */
	for( i = 0; i < gs_stFileSystemManager.dwClusterLinkAreaSize; i++){
		dwCurrentSectorOffset = (dwSectorOffset + i) % gs_stFileSystemManager.dwClusterLinkAreaSize;
		dwClusterOffset = (dwCurrentSectorOffset == gs_stFileSystemManager.dwClusterLinkAreaSize - 1 )? \
				gs_stFileSystemManager.dwTotalClusterCount % 128 : 128;
		/* 한 섹터 읽기 */
		if(kReadClusterLinkTable(dwCurrentSectorOffset, gs_vbTempBuffer)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return FILESYSTEM_LASTCLUSTER;
		}
		for(j = 0; j < dwClusterOffset; j++){
			if(((DWORD*) gs_vbTempBuffer)[ j ]==FILESYSTEM_FREECLUSTER){
				gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;
				kUnlock(&(gs_stFileSystemManager.stMutex));
				return (dwSectorOffset * 128 + j);

			}
		}
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return -1;
}
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData)
{
	DWORD dwSectorOffset;
	if(gs_stFileSystemManager.bMounted == FALSE)
		return FALSE;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */
	dwSectorOffset = dwClusterIndex / 128;
	if(kReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer)==FALSE)
		return FALSE;
	((DWORD*)gs_vbTempBuffer)[dwClusterIndex % 128] = dwData;
	if(kWriteClusterLinkTable(dwSectorOffset, gs_vbTempBuffer)==FALSE)
			return FALSE;
	return TRUE;
}
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData)
{
	DWORD dwSectorOffset;
	if(gs_stFileSystemManager.bMounted == FALSE)
		return FALSE;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */
	dwSectorOffset = dwClusterIndex / 128;
	if(dwSectorOffset >= gs_stFileSystemManager.dwClusterLinkAreaSize)
		return FALSE;
	if(kReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer)==FALSE)
		return FALSE;
	*pdwData = ((DWORD*)gs_vbTempBuffer)[dwClusterIndex % 128];

	return TRUE;
}
int kFindFreeDirectoryEntry( void )
{
	DIRECTORYENTRY* pstEntry;
	int i;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */

	/* data address에서 1클러스터를 일고 엔트리를 선형 탐색한다. */
	if(gs_stFileSystemManager.bMounted == FALSE)
		return -1;
	if(kReadCluster(0, gs_vbTempBuffer)==FALSE)
		return -1;
	pstEntry = (DIRECTORYENTRY*)gs_vbTempBuffer;
	for(i = 0;i<FILESYSTEM_MAXDIRECTORYENTRYCOUNT;i++){
		if(pstEntry[i].dwStartClusterIndex == 0){
			return i;
		}

	}
	return -1;
}
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry)
{
	DIRECTORYENTRY* pstTempEntry;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */

	/* data address에서 1클러스터를 일고 엔트리를 선형 탐색한다. */
	if((gs_stFileSystemManager.bMounted == FALSE)||(iIndex < 0)||(iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
		return FALSE;

	if(kReadCluster(0, gs_vbTempBuffer)==FALSE)
		return FALSE;
	pstTempEntry = (DIRECTORYENTRY*)gs_vbTempBuffer;
	kMemCpy(pstTempEntry+iIndex, pstEntry, sizeof(DIRECTORYENTRY));
	if(kWriteCluster(0, gs_vbTempBuffer)==FALSE)
		return FALSE;
	return TRUE;
}
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry)
{
	DIRECTORYENTRY* pstTempEntry;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */

	/* data address에서 1클러스터를 일고 엔트리를 선형 탐색한다. */
	if((gs_stFileSystemManager.bMounted == FALSE)||(iIndex < 0)||(iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
		return FALSE;

	if(kReadCluster(0, gs_vbTempBuffer)==FALSE)
		return FALSE;
	pstTempEntry = (DIRECTORYENTRY*)gs_vbTempBuffer;
	kMemCpy(pstEntry, pstTempEntry+iIndex, sizeof(DIRECTORYENTRY));
	return TRUE;
}
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry)
{
	DIRECTORYENTRY* pstTempEntry;
	int i;
	int iLength;
	/*
	 * 임시 버퍼에 대해 경쟁 상황이 벌어질 수 있는데?
	 */

	/* data address에서 1클러스터를 일고 엔트리를 선형 탐색한다. */
	if(gs_stFileSystemManager.bMounted == FALSE)
		return -1;
	if(kReadCluster(0, gs_vbTempBuffer)==FALSE)
		return -1;
	pstTempEntry = (DIRECTORYENTRY*)gs_vbTempBuffer;
	iLength = kStrlen(pcFileName);
	for(i = 0;i<FILESYSTEM_MAXDIRECTORYENTRYCOUNT;i++){
		if(kStrlen(pstTempEntry[i].vcFileName)==iLength){
			if(kMemCmp(pstTempEntry[i].vcFileName,pcFileName, iLength)==0){
				kMemCpy(pstEntry, pstTempEntry + i, sizeof(DIRECTORYENTRY));
				return i;
			}
		}

	}
	return -1;
}
void kGetFileSystemInformation(FILESYSTEMMANAGER* pstManager)
{
	kMemCpy(pstManager, &gs_stFileSystemManager, sizeof(FILESYSTEMMANAGER));
}
