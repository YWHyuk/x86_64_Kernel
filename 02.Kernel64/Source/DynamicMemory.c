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
	 * 2. 새로운 청크를 만드는 것은 단순히 새로운 노드를 추가하는 것과 거의 흡사하다.
	 * 3. 노드를 추가하고, free node의 데이터를 옮긴다.
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
	 * 만약 free chunk의 남은 공간이 충분히 작다면, <= sizeof(INUSERCHUNK);
	 * 그 free chunk를 Use in chunk로 바꾼다.
	 * 그런 경우에는, chunk간 링크 관계는 그대로..
	 * free chunk간 링크 관계는 갱신이 필요하다.
	 */
	if(pstFreeChunk->qwSize <= (qwSize +8 + sizeof(INUSECHUNK) * 2)){
		/*
		 * 남는 공간이 충분히 작은 경우
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
		 *    이 경우네는 pstInUseChunk가 존재하므로, insert_behind의 3번째 인자가 NULL일리가 없다
		 */
		/* 청크리스트 갱신*/
		bRet = insert_behind(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstFreeChunk->Chunk_LinkestList),\
				&(pstInUseChunk->Chunk_Linkestlist));
		/* 프리 청크리스트 갱신*/
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
	 * 양 옆에 아무 free chunk 가 없다면, 병합하지 않고 free chunk list에 추가하면 된다. (NULL, NULL) 제외
	 * 옆에 free chunk가 존재한다면, 단순히 병함하면 된다. size만 갱신
	 * NULL   	NULL
	 * FREE		FREE
	 * USE		USE
	 * F가 하나라도 들어가면 갱신
	 * 나머지는 왜 지멋대로인 순서가 되버리는 거지?
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

	/*병합 스타트*/
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
			//병합
		}
		//병합 안된 경우라도 갱신
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
	//청크 리스트 갱신
	//kPrintf("coalesce %q,%q\n",pstCurrent,pstNext);
	prevLinkedList = get_prev_node(&(pstCurrent->Chunk_LinkestList));
	//Print_LinkedList(&(gs_stChunkManager.Chunk_LinkedListManager), NULL);
	erase_node(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstCurrent->Chunk_LinkestList));
	erase_node(&(gs_stChunkManager.Chunk_LinkedListManager), &(pstNext->Chunk_LinkestList));
	bRet = insert_behind(&(gs_stChunkManager.Chunk_LinkedListManager),\
			&(pstCurrent->Chunk_LinkestList), prevLinkedList);
	//Print_LinkedList(&(gs_stChunkManager.Chunk_LinkedListManager), NULL);
	//프리 청크 리스트 갱신
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
	 * 이 청크가, freechunk인지, inusechunk인지 구분 가능해야함.
	 */
	FREECHUNK* pstFreeChunk = (FREECHUNK*)GETFREECHUNKADDRESS(pstLinkedList);
	kPrintf("\t\tAddres:[0x%q], Size:[%d] Magin:[%x]\n",pstFreeChunk,pstFreeChunk->qwSize,pstFreeChunk->qwMagic);
}
void Print_InUse_Chunk(LINKEDLIST* pstLinkedList)
{
	/*
	 * 이 청크가, freechunk인지, inusechunk인지 구분 가능해야함.
	 */
	INUSECHUNK* pstInUseChunk = (INUSECHUNK*)GETCHUNKADDRESS(pstLinkedList);
	kPrintf("\t\tAddres:[0x%q], Size:[%d] Magin:[%x]\n",pstInUseChunk,pstInUseChunk->qwSize,pstInUseChunk->qwMagic);
}
