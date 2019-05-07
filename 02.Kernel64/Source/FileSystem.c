/*
 * FileSystem.c
 *
 *  Created on: 2019. 3. 6.
 *      Author: DEWH
 */
#include "FileSystem.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Console.h"
//�ϵ� ��ũ ��� ���õ� �Լ� ������ ����
fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;
static FILESYSTEMMANAGER gs_stFileSystemManager;
/* �� Ŭ�����͸� ���� ���� */
static BYTE gs_vbTempBuffer[FILESYSTEM_SECTORSPERCLUSTER * 512];
BOOL kInitializeFileSystem(void)
{
	/*
	 * ����� �ܺ� ��ġ�� �˻�...
	 */
	kMemSet(&(gs_stFileSystemManager), 0, sizeof(FILESYSTEMMANAGER));
	kInitializeMutex(&(gs_stFileSystemManager.stMutex));
	/* �ϵ� ��ũ�� �˻� �����Ѵٸ�, �ܺ� ��ġ ���� �Լ��� ����  */
	if(kInitializeHDD()==TRUE){
		gs_pfReadHDDInformation = kReadHDDInformation;
		gs_pfReadHDDSector = kReadHDDSector;
		gs_pfWriteHDDSector= kWriteHDDSector;
	}else{
		return FALSE;
	}
	/* ���� �ý��� ���� */
	if( kMount() == FALSE)
	{
		return FALSE;
	}
	/* �ڵ��� ���� ������ �Ҵ� */
	gs_stFileSystemManager.pstHandlePool = \
			(FILE*) kMalloc(FILESYSTEM_HANDLE_MAXCOUNT*sizeof(FILE));
	if(gs_stFileSystemManager.pstHandlePool == NULL){
		gs_stFileSystemManager.bMounted = FALSE;
		return FALSE;
	}
	kMemSet(gs_stFileSystemManager.pstHandlePool, 0, \
			FILESYSTEM_HANDLE_MAXCOUNT*sizeof(FILE));
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
	 *  ������ ���� cluster_count�� ���غ���
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
	/* filesystemmanger�� ���� �����Ϳ� �����Ѵ�. */
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
/* Ŭ������ ��ũ ���̺� ���� �Լ� */
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
/* Ŭ������ ���� �Լ� */
BOOL kReadCluster(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfReadHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwDataAreaStartAddress + \
				dwOffset * FILESYSTEM_SECTORSPERCLUSTER, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}
BOOL kWriteCluster(DWORD dwOffset, BYTE* pbBuffer)
{
	return gs_pfWriteHDDSector(TRUE, TRUE, gs_stFileSystemManager.dwDataAreaStartAddress + \
			dwOffset * FILESYSTEM_SECTORSPERCLUSTER, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}
DWORD kFindFreeCluster( void )
{
	DWORD dwLinkCountInSector;
	DWORD dwLastSectorOffset,dwCurrentSectorOffset;
	DWORD i,j;
	if(gs_stFileSystemManager.bMounted==FALSE)
		return FILESYSTEM_LASTCLUSTER;
	dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;
	for(i = 0;i<gs_stFileSystemManager.dwClusterLinkAreaSize;i++){
		if((dwLastSectorOffset + i)==(gs_stFileSystemManager.dwClusterLinkAreaSize - 1))
			dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;
		else
			dwLinkCountInSector = 128;
		dwCurrentSectorOffset = (dwLastSectorOffset + i) % gs_stFileSystemManager.dwClusterLinkAreaSize;
		if(kReadClusterLinkTable(dwCurrentSectorOffset, gs_vbTempBuffer)==FALSE)
			return FILESYSTEM_LASTCLUSTER;
		for(j=0;j<dwLinkCountInSector;j++){
			if(((DWORD*)gs_vbTempBuffer)[j]==FILESYSTEM_FREECLUSTER)
				break;
		}
		if(j!= dwLinkCountInSector){
			gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;
			return (dwCurrentSectorOffset * 128) +j;
		}
	}
	return FILESYSTEM_LASTCLUSTER;
}
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData)
{
	DWORD dwSectorOffset;
	if(gs_stFileSystemManager.bMounted == FALSE)
		return FALSE;
	/*
	 * �ӽ� ���ۿ� ���� ���� ��Ȳ�� ������ �� �ִµ�?
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
	 * �ӽ� ���ۿ� ���� ���� ��Ȳ�� ������ �� �ִµ�?
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
	/* data address���� 1Ŭ�����͸� �ϰ� ��Ʈ���� ���� Ž���Ѵ�. */
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
	/* data address���� 1Ŭ�����͸� �ϰ� ��Ʈ���� ���� Ž���Ѵ�. */
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
	/* data address���� 1Ŭ�����͸� �ϰ� ��Ʈ���� ���� Ž���Ѵ�. */
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
	 * �ӽ� ���ۿ� ���� ���� ��Ȳ�� ������ �� �ִµ�?
	 */

	/* data address���� 1Ŭ�����͸� �ϰ� ��Ʈ���� ���� Ž���Ѵ�. */
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
/*==========================================================================
 * 							High Level Functions
 * ==========================================================================*/
/**/
static void* kAllocateFileDirectoryHandle(void)
{
	int i;
	FILE* pstFile;
	pstFile = gs_stFileSystemManager.pstHandlePool;
	for(i =0;i<FILESYSTEM_HANDLE_MAXCOUNT;i++){
		if(pstFile->bType == FILESYSTEM_TYPE_FREE){
			pstFile->bType = FILESYSTEM_TYPE_FILE;
			return pstFile;
		}
		pstFile++;
	}
	return NULL;
}
static void kFreeFileDirectoryHandle(FILE* pstFile)
{
	kMemSet(pstFile, 0, sizeof(FILE));
	pstFile->bType = FILESYSTEM_TYPE_FREE;
	return;
}
static BOOL kCreateFile(const char* pcFileName, DIRECTORYENTRY* pstEntry,int* piDirectoryEntryIndex)
{
	/* �� ������ ����
	 * Input: pcFileName
	 * Output: pstEntry, piDirectoryEntryIndex
	 *
	 * 1. �� ���丮 ��Ʈ���� ã�� �Ҵ�.
	 * 2. �� Ŭ�����͸� ã�� �Ҵ�.
	 */
	DWORD dwCluster;
	dwCluster = kFindFreeCluster();
	if((dwCluster == FILESYSTEM_LASTCLUSTER )|| (kSetClusterLinkData(dwCluster, \
			FILESYSTEM_LASTCLUSTER)==FALSE)){
		return FALSE;
	}
	*piDirectoryEntryIndex = kFindFreeDirectoryEntry();
	if(*piDirectoryEntryIndex == -1){
		kSetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
		return FALSE;
	}
	kMemCpy(pstEntry->vcFileName,pcFileName,kStrlen(pcFileName) + 1);
	pstEntry->dwStartClusterIndex = dwCluster;
	pstEntry->dwFileSize = 0;
	if(kSetDirectoryEntryData(*piDirectoryEntryIndex, pstEntry) == FALSE){
		kSetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
		return FALSE;
	}
	return TRUE;
}
static BOOL kFreeClusterUntilEnd(DWORD dwClusterIndex)
{
	DWORD dwCurrentClusterIndex;
	DWORD dwNextClusterIndex;
	dwCurrentClusterIndex = dwClusterIndex;
	while(dwCurrentClusterIndex != FILESYSTEM_LASTCLUSTER){
		if(kGetClusterLinkData(dwCurrentClusterIndex, &dwNextClusterIndex)==FALSE)
			return FALSE;
		if(kSetClusterLinkData(dwCurrentClusterIndex, FILESYSTEM_FREECLUSTER)==FALSE)
			return FALSE;
		dwCurrentClusterIndex = dwNextClusterIndex;
	}
	return TRUE;
}
/* ��Ʈ ���丮���� ���丮 ��Ʈ�� ���� ���� */
static BOOL kUpdateDirectoryEntry(FILEHANDLE* pstFileHandle)
{
	DIRECTORYENTRY stEntry;
	if((pstFileHandle == NULL)||\
			(kGetDirectoryEntryData(pstFileHandle->iDirectoryEntryoffset, &stEntry)==FALSE))
		return FALSE;
	stEntry.dwFileSize = pstFileHandle->dwFileSize;
	stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;
	if(kSetDirectoryEntryData(pstFileHandle->iDirectoryEntryoffset, &stEntry)==FALSE)
		return FALSE;
	return TRUE;
}
FILE* kOpenFile(const char* pcFileName, const char* pcMode)
{
	/*r ���� r/w ���� */
	DIRECTORYENTRY stEntry;
	FILE* pstFile;
	DWORD second_Cluster;
	int dirOffset;
	int len;
	/* ���� �̸� �˻� */
	len = kStrlen(pcFileName);
	if((len == 0) || (len - 1 > sizeof(stEntry.vcFileName)))
		return FALSE;
	/* ����ȭ ��ü */
	kLock(&(gs_stFileSystemManager.stMutex));
	dirOffset = kFindDirectoryEntry(pcFileName, &stEntry);
	if(dirOffset==-1){
		if(pcMode[0]=='r'){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
		if(kCreateFile(pcFileName, &stEntry, &dirOffset)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
	}else if(pcMode[0]=='w'){

		if(kGetClusterLinkData(stEntry.dwStartClusterIndex, &second_Cluster)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
		if(kSetClusterLinkData(stEntry.dwStartClusterIndex, FILESYSTEM_LASTCLUSTER)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
		if(kFreeClusterUntilEnd(second_Cluster)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
		stEntry.dwFileSize = 0;
		if(kSetDirectoryEntryData(dirOffset, &stEntry)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return NULL;
		}
	}
	pstFile = kAllocateFileDirectoryHandle();
	if(pstFile == NULL){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		return NULL;
	}
	pstFile->bType = FILESYSTEM_TYPE_FILE;
	pstFile->stFileHandle.iDirectoryEntryoffset = dirOffset;
	pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
	pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwCurrentOffset = 0;
	if(pcMode[0]=='a'){
		kSeekFile(pstFile,0,FILESYSTEM_SEEK_END);
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return pstFile;
}
DWORD kReadFile(void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile)
{
	/* ���� �������� ��ġ�� ������ ũ��, �׸��� ��û�� ����Ʈ ���� ���Ͽ� ������ �о�� �� ũ�⸦ ���
	 * ���� Ŭ�����͸� �о Ŭ�������� ������ ��û�� ���۷� ����
	 * ���� ����Ʈ ���� �о�� �� ����Ʈ ��, ���� ������ ����
	 * ���� Ŭ�������� ������ �����ߴٸ�, ����Ŭ�����ͷ� �̵�
	 * ���� �ڵ��� ���� Ŭ�����Ϳ� ���� Ŭ������ ������ ����
	 * ���� ����Ʈ ���� ������ ������ ��ȯ */
	FILEHANDLE* fileHandle;
	DWORD totalByte;
	DWORD readByte;
	DWORD clusterOffset;
	DWORD copySize;
	DWORD nextClusterIndex;
	if((pstFile == NULL)||(pstFile->bType != FILESYSTEM_TYPE_FILE))
		return 0;
	fileHandle = &(pstFile->stFileHandle);
	if((fileHandle->dwCurrentOffset == fileHandle->dwFileSize)||\
			(fileHandle->dwCurrentClusterIndex==FILESYSTEM_LASTCLUSTER))
		return 0;
	totalByte = MIN(dwSize*dwCount,fileHandle->dwFileSize - fileHandle->dwCurrentOffset);
	readByte = 0;
	kLock(&(gs_stFileSystemManager.stMutex));
	while(readByte!=totalByte){
		if(kReadCluster(fileHandle->dwCurrentClusterIndex,gs_vbTempBuffer)==FALSE)
			break;
		clusterOffset = fileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;
		copySize = MIN(totalByte-readByte,FILESYSTEM_CLUSTERSIZE-clusterOffset);
		kMemCpy((char*)pvBuffer + readByte, gs_vbTempBuffer + clusterOffset, copySize);
		readByte += copySize;
		fileHandle->dwCurrentOffset += copySize;

		if((fileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE)==0){
			if(kGetClusterLinkData(fileHandle->dwCurrentClusterIndex, \
					&nextClusterIndex)==FALSE)
				break;
			fileHandle->dwPreviousClusterIndex = fileHandle->dwCurrentClusterIndex;
			fileHandle->dwCurrentClusterIndex = nextClusterIndex;
		}
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return (readByte/dwSize);
}
DWORD kWriteFile(const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile)
{
	FILEHANDLE* fileHandle;
	DWORD totalByte;
	DWORD writeByte;

	DWORD clusterOffset;
	DWORD copySize;

	DWORD nextClusterIndex;
	DWORD newClusterIndex;

	if((pstFile == NULL)||(pstFile->bType != FILESYSTEM_TYPE_FILE))
		return 0;
	fileHandle = &(pstFile->stFileHandle);
	totalByte = dwSize*dwCount;
	kLock(&(gs_stFileSystemManager.stMutex));
	writeByte = 0;
	while(writeByte!=totalByte){
		if(fileHandle->dwCurrentClusterIndex==FILESYSTEM_LASTCLUSTER){
			newClusterIndex = kFindFreeCluster();
			if(newClusterIndex == FILESYSTEM_LASTCLUSTER)
				break;
			if(kSetClusterLinkData(newClusterIndex, FILESYSTEM_LASTCLUSTER)==FALSE)
				break;
			if(kSetClusterLinkData(fileHandle->dwPreviousClusterIndex, newClusterIndex)==FALSE){
				kSetClusterLinkData(newClusterIndex, FILESYSTEM_FREECLUSTER);
				break;
			}
			fileHandle->dwCurrentClusterIndex = newClusterIndex;
			kMemSet(gs_vbTempBuffer,0,FILESYSTEM_CLUSTERSIZE);
		}else if(((fileHandle->dwCurrentOffset %FILESYSTEM_CLUSTERSIZE)!=0)||\
				((totalByte - writeByte)<FILESYSTEM_CLUSTERSIZE)){
			if(kReadCluster(fileHandle->dwCurrentClusterIndex, gs_vbTempBuffer)==FALSE)
				break;
		}
		clusterOffset = fileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;
		copySize = MIN(FILESYSTEM_CLUSTERSIZE - clusterOffset,totalByte - writeByte);
		kMemCpy(gs_vbTempBuffer+clusterOffset,(char*)pvBuffer + writeByte,copySize);
		if(kWriteCluster(fileHandle->dwCurrentClusterIndex, gs_vbTempBuffer)==FALSE)
			break;
		writeByte += copySize;
		fileHandle->dwCurrentOffset += copySize;
		if((fileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) == 0){
			if(kGetClusterLinkData(fileHandle->dwCurrentClusterIndex, \
					&nextClusterIndex)==FALSE)
				break;
			fileHandle->dwPreviousClusterIndex = fileHandle->dwCurrentClusterIndex;
			fileHandle->dwCurrentClusterIndex = nextClusterIndex;
		}
	}
	if(fileHandle->dwFileSize < fileHandle->dwCurrentOffset){
		fileHandle->dwFileSize = fileHandle->dwCurrentOffset;
		kUpdateDirectoryEntry(fileHandle);
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	//kPrintf("prev:%d current:%x next:%x\n",fileHandle->dwPreviousClusterIndex,\
						fileHandle->dwCurrentClusterIndex,nextClusterIndex);
	return writeByte;
}
int kSeekFile(FILE* pstFile, int iOffset, int iOrigin)
{
	FILEHANDLE* fileHandle;
	DWORD realOffset;
	DWORD lastClusterOffset;
	DWORD currentClusterOffset;
	DWORD targetClusterOffset;
	DWORD clusterMoveCount;
	DWORD startClusterIndex;

	DWORD currentClusterIndex;
	DWORD prevClusterIndex;

	int i;
	if((pstFile == NULL)||(pstFile->bType != FILESYSTEM_TYPE_FILE))
		return 1;
	fileHandle = &(pstFile->stFileHandle);
	/* realOffset ��� ���� */
	switch(iOrigin){
	case FILESYSTEM_SEEK_SET:
		if(iOffset<=0)
			realOffset = 0;
		else
			realOffset = iOffset;
		break;
	case FILESYSTEM_SEEK_CUR:
		if((iOffset<0) && (fileHandle->dwCurrentOffset <= (DWORD) -iOffset))
			realOffset = 0;
		else
			realOffset = fileHandle->dwCurrentOffset + iOffset;
		break;
	case FILESYSTEM_SEEK_END:
		if((iOffset<0) && (fileHandle->dwFileSize <= (DWORD) -iOffset))
			realOffset = 0;
		else
			realOffset = fileHandle->dwFileSize + iOffset;
		break;
	}
	/* ���� ��������ġ�� ���� 3���� ������� ������ �̵�
	 * 1. ���� �����Ͱ� ��ġ�ϴ� Ŭ�����Ͱ� ������ Ŭ������ ���Ŀ� ��ġ
	 * 	�������� �� ����? A)�Ƹ��� prev�� ó���ؾߵǱ� ����.
	 * 2. ���� �����Ͱ� ��ġ�ϴ� Ŭ�����Ͱ� ���� Ŭ������ ���Ŀ� ��ġ
	 * 3. ���� �����Ͱ� ��ġ�ϴ� Ŭ�����Ͱ� ���� Ŭ������ ������ ��ġ
	 * */
	lastClusterOffset = fileHandle->dwFileSize / FILESYSTEM_CLUSTERSIZE;
	currentClusterOffset = fileHandle->dwCurrentOffset / FILESYSTEM_CLUSTERSIZE;
	targetClusterOffset = realOffset / FILESYSTEM_CLUSTERSIZE;
	if(targetClusterOffset > lastClusterOffset){
		clusterMoveCount = lastClusterOffset - currentClusterOffset;
		startClusterIndex = fileHandle->dwCurrentClusterIndex;
	}else if(targetClusterOffset >= currentClusterOffset){
		clusterMoveCount = targetClusterOffset - currentClusterOffset;
		startClusterIndex = fileHandle->dwCurrentClusterIndex;
	}else{
		clusterMoveCount = targetClusterOffset;
		startClusterIndex = fileHandle->dwStartClusterIndex;
	}
	kLock(&(gs_stFileSystemManager.stMutex));
	currentClusterIndex = startClusterIndex;
	for(i =0;i<clusterMoveCount;i++){
		prevClusterIndex = currentClusterIndex;
		if(prevClusterIndex == FILESYSTEM_LASTCLUSTER)
			break;
		if(kGetClusterLinkData(prevClusterIndex, &currentClusterIndex)==FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			kPrintf("1 target %d, last %d\n",targetClusterOffset,lastClusterOffset);
			kPrintf("origin:%d movecount:%d offset:%d\n",iOrigin,clusterMoveCount,iOffset);
			kPrintf("prevClusterIndex: %x\n",prevClusterIndex);
			return 2;
		}
	}
	if(clusterMoveCount>0){
		fileHandle->dwCurrentClusterIndex = currentClusterIndex;
		fileHandle->dwPreviousClusterIndex = prevClusterIndex;
	}else if(startClusterIndex == fileHandle->dwStartClusterIndex){
		fileHandle->dwCurrentClusterIndex = fileHandle->dwStartClusterIndex;
		fileHandle->dwPreviousClusterIndex = fileHandle->dwStartClusterIndex;
	}
	/* ���� �����͸� �����ϰ� ���� �������� ������ ũ�⸦ �Ѿ��ٸ�, ������ �κ��� 0���� ä��� ������ ũ�⸦ �ø� */
	if(lastClusterOffset < targetClusterOffset){
		fileHandle->dwCurrentOffset = fileHandle->dwFileSize;
		kUnlock(&(gs_stFileSystemManager.stMutex));
		if(kWriteZero(pstFile, realOffset - fileHandle->dwFileSize) == FALSE){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			kPrintf("2 target %d, last %d\n",targetClusterOffset,lastClusterOffset);

			kPrintf("origin:%d movecount:%d offset:%d\n",iOrigin,clusterMoveCount,iOffset);
			kPrintf("prevClusterIndex: %x\n",prevClusterIndex);
			return 3;
		}
	}
	fileHandle->dwCurrentOffset = realOffset;
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return 0;
}
int kCloseFile(FILE* pstFile)
{
	if((pstFile == NULL)||(pstFile->bType != FILESYSTEM_TYPE_FILE))
		return -1;
	kFreeFileDirectoryHandle(pstFile);
	return 0;
}
int kRemoveFile(const char* pcFileName)
{
	DIRECTORYENTRY entry;
	int directoryEntryOffset;
	int nameLength;
	nameLength = kStrlen(pcFileName);
	/* ���� �̸� �˻� */
	if(nameLength> (sizeof(entry.vcFileName)-1) || nameLength==0)
		return -1;
	kLock(&(gs_stFileSystemManager.stMutex));
	directoryEntryOffset = kFindDirectoryEntry(pcFileName, &entry);
	if(directoryEntryOffset == -1){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("find directory entry fail\n");
		return -1;
	}
	if(kIsFileOpened(&entry)==TRUE){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("file is opened\n");
		return -1;
	}
	if(kFreeClusterUntilEnd(entry.dwStartClusterIndex)==FALSE){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("freeing clusters failed\n");
		return -1;
	}
	kMemSet(&entry, 0, sizeof(DIRECTORYENTRY));
	if(kSetDirectoryEntryData(directoryEntryOffset, &entry)==FALSE){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("Update directory entry failed\n");
		return -1;
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return 0;
}
DIR* kOpenDirectory(const char* pcDirectoryName)
{
	DIR* directory;
	DIRECTORYENTRY* directoryBuffer;
	kLock(&(gs_stFileSystemManager.stMutex));
	directory = kAllocateFileDirectoryHandle();
	if(directory == NULL){
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("handle alloc failed\n");
		return NULL;
	}
	directoryBuffer = (DIRECTORYENTRY*)kMalloc(FILESYSTEM_CLUSTERSIZE);
	if(directoryBuffer==NULL){
		kFreeFileDirectoryHandle(directory);
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("dynamic alloc failed\n");
		return NULL;
	}
	if(kReadCluster(0, (BYTE*)directoryBuffer)==FALSE){
		kFree(directoryBuffer);
		kFreeFileDirectoryHandle(directory);
		kUnlock(&(gs_stFileSystemManager.stMutex));
		kPrintf("read cluster failed\n");
		return NULL;
	}

	directory->bType = FILESYSTEM_TYPE_DIRECTORY;
	directory->stDirectoryHandle.iCurrentOffset = 0;
	directory->stDirectoryHandle.pstDirectoryBuffer = directoryBuffer;
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return directory;
}
struct kDirectoryEntryStruct* kReadDirectory(DIR* pstDirectory)
{
	DIRECTORYHANDLE* directoryHandle;
	DIRECTORYENTRY* directoryEntry;
	if((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY)){
		return NULL;
	}
	directoryHandle = &(pstDirectory->stDirectoryHandle);
	if((directoryHandle->iCurrentOffset<0) || (directoryHandle->iCurrentOffset>= FILESYSTEM_MAXDIRECTORYENTRYCOUNT)){
		return NULL;
	}
	kLock(&(gs_stFileSystemManager.stMutex));
	directoryEntry = directoryHandle->pstDirectoryBuffer;
	while(directoryHandle->iCurrentOffset < FILESYSTEM_MAXDIRECTORYENTRYCOUNT){
		if(directoryEntry[directoryHandle->iCurrentOffset].dwStartClusterIndex!=0){
			kUnlock(&(gs_stFileSystemManager.stMutex));
			return &(directoryEntry[directoryHandle->iCurrentOffset++]);
		}
		directoryHandle->iCurrentOffset++;
	}
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return NULL;
}
void kRewindDirectory(DIR* pstDirectory)
{
	DIRECTORYHANDLE* directoryHandle;
	if((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY)){
		return;
	}
	directoryHandle = &(pstDirectory->stDirectoryHandle);
	kLock(&(gs_stFileSystemManager.stMutex));
	directoryHandle->iCurrentOffset = 0;
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return;
}
int kCloseDirectory(DIR* pstDirectory)
{
	DIRECTORYHANDLE* directoryHandle;
	if((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY)){
		return -1;
	}
	directoryHandle = &(pstDirectory->stDirectoryHandle);
	kLock(&(gs_stFileSystemManager.stMutex));
	kFreeFileDirectoryHandle(pstDirectory);
	kUnlock(&(gs_stFileSystemManager.stMutex));
	return 0;
}
BOOL kWriteZero(FILE* pstFile, DWORD dwCount)
{
	BYTE* buffer;
	DWORD remainCount;
	DWORD writeCount;
	if(pstFile == NULL)
		return FALSE;
	buffer = (BYTE*) kMalloc(FILESYSTEM_CLUSTERSIZE);
	if(buffer == NULL)
		return FALSE;
	kMemSet(buffer, 0, FILESYSTEM_CLUSTERSIZE);
	remainCount = dwCount;
	while(remainCount!=0){
		writeCount = MIN(remainCount,FILESYSTEM_CLUSTERSIZE);
		if(kWriteFile(buffer,1,writeCount,pstFile)!=writeCount){
			kFree(buffer);
			kPrintf("writing count is not correct\n");
			return FALSE;
		}
		remainCount -= writeCount;
	}
	kFree(buffer);
	return TRUE;
}
BOOL kIsFileOpened(const DIRECTORYENTRY* pstEntry)
{
	int i;
	FILE* pstFile;
	pstFile = gs_stFileSystemManager.pstHandlePool;
	for(i =0;i<FILESYSTEM_HANDLE_MAXCOUNT;i++){
		if((pstFile[i].bType == FILESYSTEM_TYPE_FILE)&&\
				(pstFile[i].stFileHandle.dwStartClusterIndex ==\
						pstEntry->dwStartClusterIndex))
			return TRUE;
	}
	return FALSE;
}
