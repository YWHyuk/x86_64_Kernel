/*
 * DynamicMemory.c
 *
 *  Created on: 2019. 3. 4.
 *      Author: DEWH
 */
#include "DynamicMemory.h"
CHUNKMANAGER gs_stChunkManager;
void kInitializeDynamicMemory()
{
	FREECHUNK* pstFreeChunk = (FREECHUNK*) DYNAMICMEMORY_START_ADDRESS;
	pstFreeChunk->qwMagic = CHUNK_TYPE_FREE;
	pstFreeChunk->qwSize =0x100000 * 47;
	InitializeLinkedListManger(&(gs_stChunkManager.Chunk_LinkedListManager));
	InitializeLinkedListManger(&(gs_stChunkManager.Free_Chunk_LinkedListManager));
	push_front(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstFreeChunk->Chunk_LinkestList));
	push_front(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(pstFreeChunk->Free_Chunk_LinkedList));
}
void* kMalloc(QWORD qwSize)
{
	/*
	 * 1. select one free chunk(first fit,best fit);
	 * 2. ���ο� ûũ�� ����� ���� �ܼ��� ���ο� ��带 �߰��ϴ� �Ͱ� ���� ����ϴ�.
	 * 3. ��带 �߰��ϰ�, free node�� �����͸� �ű��.
	 */
	FREECHUNK* pstFreeChunk;
	INUSECHUNK* pstInUseChunk;
	QWORD qwRetAddress;
	int iIndex;
	BOOL bPrevious;
	BOOL bRet;
	LINKEDLIST* pstLinkedList;
	bPrevious = kLockForSystemData();
	pstFreeChunk = kSelectFreeChunk(qwSize);
	if(pstFreeChunk == NULL){
		kUnLockForSystemData(bPrevious);
		//kPrintf("kMalloc Failed...\n");
		return NULL;
	}
	/*
	 * ���� free chunk�� ���� ������ ����� �۴ٸ�, <= sizeof(INUSERCHUNK);
	 * �� free chunk�� Use in chunk�� �ٲ۴�.
	 * �׷� ��쿡��, chunk�� ��ũ ����� �״��..
	 * free chunk�� ��ũ ����� ������ �ʿ��ϴ�.
	 */
	if(pstFreeChunk->qwSize <= (qwSize +8 + sizeof(INUSECHUNK) * 2)){
		/*
		 * ���� ������ ����� ���� ���
		*/
		erase_node(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(pstFreeChunk->Free_Chunk_LinkedList));
		pstInUseChunk = (INUSECHUNK*) pstFreeChunk;

		qwRetAddress = ((QWORD)pstInUseChunk) + sizeof(INUSECHUNK);
	}else{
		pstLinkedList = pstFreeChunk->Free_Chunk_LinkedList.Prev_LinkedList;
		qwRetAddress = ((QWORD)pstFreeChunk) + sizeof(INUSECHUNK);
		pstInUseChunk = (INUSECHUNK*) pstFreeChunk;//INUSE
		pstFreeChunk = (FREECHUNK*)(qwRetAddress + qwSize);//FREE

		/*
		 *        fk -->        fk -->
		 *    �� ���״� pstInUseChunk�� �����ϹǷ�, insert_behind�� 3��° ���ڰ� NULL�ϸ��� ����
		 */
		/* ûũ����Ʈ ����*/
		bRet = insert_behind(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstFreeChunk->Chunk_LinkestList),\
				&(pstInUseChunk->Chunk_Linkestlist));
		/* ���� ûũ����Ʈ ����*/
		erase_node(&(gs_stChunkManager.Free_Chunk_LinkedListManager),\
				&(((FREECHUNK*)pstInUseChunk)->Free_Chunk_LinkedList));
		bRet = insert_behind(&(gs_stChunkManager.Free_Chunk_LinkedListManager),\
				&(pstFreeChunk->Free_Chunk_LinkedList), pstLinkedList);
		if(bRet == FALSE)
			kPrintf("kMalloc insert error...\n");
		pstFreeChunk->qwSize = pstInUseChunk->qwSize - (qwSize + sizeof(INUSECHUNK));
		//kPrintf("Remain:%d\n",pstFreeChunk->qwSize);
		pstFreeChunk->qwMagic = CHUNK_TYPE_FREE;

		pstInUseChunk->qwSize = qwSize;
		pstInUseChunk->qwMagic = CHUNK_TYPE_USE;
	}
	pstInUseChunk->qwMagic = CHUNK_TYPE_USE;
	kUnLockForSystemData(bPrevious);
	kMemSet(qwRetAddress, 0, pstInUseChunk->qwSize);
	return (void*)qwRetAddress;
}
static FREECHUNK* kSelectFreeChunk(QWORD qwSize)
{
	/*
	 * First fit is selected
	 */
	LINKEDLIST* LinkedList;
	FREECHUNK* FreeChunk;
	for(LinkedList = (LINKEDLIST*)front(&(gs_stChunkManager.Free_Chunk_LinkedListManager));\
	LinkedList!=NULL;\
	LinkedList = get_next_node(LinkedList)){
		FreeChunk = (FREECHUNK*)GETFREECHUNKADDRESS(LinkedList);
		if(FreeChunk->qwSize >= qwSize + sizeof(INUSECHUNK) && FreeChunk->qwMagic == CHUNK_TYPE_FREE)
			return FreeChunk;
	}
	return NULL;
}
void kFree(void* pvAddress)
{
	/*
	 * �� ���� �ƹ� free chunk �� ���ٸ�, �������� �ʰ� free chunk list�� �߰��ϸ� �ȴ�. (NULL, NULL) ����
	 * ���� free chunk�� �����Ѵٸ�, �ܼ��� �����ϸ� �ȴ�. size�� ����
	 * NULL   	NULL
	 * FREE		FREE
	 * USE		USE
	 * F�� �ϳ��� ���� ����
	 * �������� �� ���ڴ���� ������ �ǹ����� ����?
	 */
	INUSECHUNK* pstInUseChunk;
	INUSECHUNK* pstPrev, *pstNext;
	FREECHUNK *pstCurrentNode, *pstNextNode;
	LINKEDLIST* pstTempLinkedList;
	LINKEDLIST* pstLinkedList;

	int iIndex;
	void* pvPrev, *pvNext;
	BOOL bIsPrevFree = FALSE, bIsNextFree = FALSE;
	BOOL bPrevious;
	BOOL bRet;
	bPrevious = kLockForSystemData();
	pstInUseChunk = (INUSECHUNK*)(((QWORD)pvAddress) - sizeof(INUSECHUNK));
	if(pstInUseChunk->qwMagic != CHUNK_TYPE_USE){
		kUnLockForSystemData(bPrevious);
		kPrintf("It is not used chunk\n");
		return;
	}
	for(pstTempLinkedList = (pstInUseChunk->Chunk_Linkestlist.Prev_LinkedList);\
	pstTempLinkedList!=NULL; pstTempLinkedList= get_prev_node(pstTempLinkedList)){
		pstPrev = (INUSECHUNK*)GETCHUNKADDRESS(pstTempLinkedList);
		if(pstPrev->qwMagic != CHUNK_TYPE_USE){
			break;
		}
	}
	if(pstInUseChunk == (INUSECHUNK*)GETCHUNKADDRESS(front(&(gs_stChunkManager.Chunk_LinkedListManager)))){
		bRet = push_front(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(((FREECHUNK*)pstInUseChunk)->Free_Chunk_LinkedList));
		if(bRet==FALSE)
			kPrintf("push front failed...\n");
		//kPrintf("special push front:[0x%q]\n",pstInUseChunk);
	}else if(pstPrev->qwMagic == CHUNK_TYPE_USE){
		bRet = push_front(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(((FREECHUNK*)pstInUseChunk)->Free_Chunk_LinkedList));
		if(bRet==FALSE)
			kPrintf("push front failed...\n");
		//kPrintf("push front:[0x%q]\n",pstInUseChunk);
	}else{
		bRet = insert_behind(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(((FREECHUNK*)pstInUseChunk)->Free_Chunk_LinkedList)\
				, &(((FREECHUNK*)pstPrev)->Free_Chunk_LinkedList));
		if(bRet==FALSE)
			kPrintf("insert behind failed...\n");
		//kPrintf("insert:[0x%q],[0x%q]\n",pstPrev,pstInUseChunk);
	}
	pstInUseChunk->qwMagic = CHUNK_TYPE_UNREADY;
	kUnLockForSystemData(bPrevious);

	/*���� ��ŸƮ*/
	bPrevious = kLockForSystemData();
	debug();
	kUnLockForSystemData(bPrevious);
	return;
}
void debug(){
	BOOL bPrevious;
	LINKEDLIST* pstLinkedList;
	FREECHUNK* pstCurrentNode;
	FREECHUNK* pstNextNode;
	bPrevious = kLockForSystemData();
	if(count(&(gs_stChunkManager.Chunk_LinkedListManager))<2){
		kUnLockForSystemData(bPrevious);
		return;
	}
	pstLinkedList = front(&(gs_stChunkManager.Chunk_LinkedListManager));
	while(1){
		if(get_next_node(pstLinkedList)==NULL)
			break;
		pstCurrentNode = (FREECHUNK*) GETCHUNKADDRESS(pstLinkedList);
		pstNextNode = (FREECHUNK*) GETCHUNKADDRESS(get_next_node(pstLinkedList));
		//kPrintf("[0x%q]:0x%q [0x%q]:0x%q\n",pstCurrentNode,pstCurrentNode->qwMagic,pstNextNode,pstNextNode->qwMagic);
		if((pstCurrentNode->qwMagic!=CHUNK_TYPE_USE) && (pstNextNode->qwMagic!=CHUNK_TYPE_USE)){
			if(coalesce(pstCurrentNode, pstNextNode)==FALSE)
				kPrintf("Something wrong...\n");
			continue;
			//����
		}
		//���� �ȵ� ���� ����
		if(pstCurrentNode->qwMagic == CHUNK_TYPE_UNREADY)
			pstCurrentNode->qwMagic = CHUNK_TYPE_FREE;
		if(pstNextNode->qwMagic == CHUNK_TYPE_UNREADY)
			pstNextNode->qwMagic = CHUNK_TYPE_FREE;
		pstLinkedList = get_next_node(pstLinkedList);
	}
	kUnLockForSystemData(bPrevious);
	return;
}
BOOL coalesce(FREECHUNK* pstCurrent,FREECHUNK* pstNext)
{
	LINKEDLIST* prevLinkedList;
	LINKEDLIST* prevFreeLinkedList;
	BOOL bRet;
	//kClearScreen();
	//kPrintf("coalesce start\n");
	//Print_LinkedList(&(gs_stChunkManager.Free_Chunk_LinkedListManager), Print_Free_Chunk);
	pstCurrent->qwSize += (pstNext->qwSize + sizeof(FREECHUNK));
	pstCurrent->qwMagic = CHUNK_TYPE_FREE;
	//ûũ ����Ʈ ����
	//kPrintf("coalesce %q,%q\n",pstCurrent,pstNext);
	prevLinkedList = get_prev_node(&(pstCurrent->Chunk_LinkestList));
	//Print_LinkedList(&(gs_stChunkManager.Chunk_LinkedListManager), NULL);
	erase_node(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstCurrent->Chunk_LinkestList));
	erase_node(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstNext->Chunk_LinkestList));
	bRet = insert_behind(&(gs_stChunkManager.Chunk_LinkedListManager),\
			&(pstCurrent->Chunk_LinkestList), prevLinkedList);
	//Print_LinkedList(&(gs_stChunkManager.Chunk_LinkedListManager), NULL);
	//���� ûũ ����Ʈ ����
	prevLinkedList = get_prev_node(&(pstCurrent->Free_Chunk_LinkedList));
	erase_node(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(pstCurrent->Free_Chunk_LinkedList));
	erase_node(&(gs_stChunkManager.Free_Chunk_LinkedListManager), &(pstNext->Free_Chunk_LinkedList));
	bRet = insert_behind(&(gs_stChunkManager.Free_Chunk_LinkedListManager),\
			&(pstCurrent->Free_Chunk_LinkedList), prevLinkedList);
	if(bRet == FALSE){
		kPrintf("Critical Error in Heap");
		return bRet;
	}

	//kClearScreen();
	//kPrintf("coalesce end\n");
	//Print_LinkedList(&(gs_stChunkManager.Free_Chunk_LinkedListManager), Print_Free_Chunk);

	return TRUE;

}
void Print_Free_Chunk(LINKEDLIST* pstLinkedList)
{
	/*
	 * �� ûũ��, freechunk����, inusechunk���� ���� �����ؾ���.
	 */
	FREECHUNK* pstFreeChunk = (FREECHUNK*)GETFREECHUNKADDRESS(pstLinkedList);
	kPrintf("\t\tAddres:[0x%q], Size:[%d] Magin:[%x]\n",pstFreeChunk,pstFreeChunk->qwSize,pstFreeChunk->qwMagic);
}
void Print_InUse_Chunk(LINKEDLIST* pstLinkedList)
{
	/*
	 * �� ûũ��, freechunk����, inusechunk���� ���� �����ؾ���.
	 */
	INUSECHUNK* pstInUseChunk = (INUSECHUNK*)GETCHUNKADDRESS(pstLinkedList);
	kPrintf("\t\tAddres:[0x%q], Size:[%d] Magin:[%x]\n",pstInUseChunk,pstInUseChunk->qwSize,pstInUseChunk->qwMagic);
}
