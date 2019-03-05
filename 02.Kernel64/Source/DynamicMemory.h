/*
 * DynamicMemory.h
 *
 *  Created on: 2019. 3. 3.
 *      Author: DEWH
 */

#ifndef __02_KERNEL64_SOURCE_DYNAMICMEMORY_H_
#define __02_KERNEL64_SOURCE_DYNAMICMEMORY_H_
#include "Types.h"
#include "LinkedList.h"
#include "Task.h"

#define CHUNK_TYPE_USE 		0x10000000
#define CHUNK_TYPE_FREE		0x20000000
#define CHUNK_TYPE_UNREADY	0x40000000
#define DYNAMICMEMORY_START_ADDRESS		((TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * TASK_MAXCOUNT)+0xFFFFF) &  0xFFFFFFFFFFF00000)
#define GETFREECHUNKADDRESS(x)			((QWORD)(x) - offsetof(FREECHUNK,Free_Chunk_LinkedList))
#define GETCHUNKADDRESS(x)				((QWORD)(x) - offsetof(FREECHUNK,Chunk_LinkestList))
#pragma pack(push,1)
typedef struct kFreeChunk{
	QWORD qwSize;
	QWORD qwMagic;
	LINKEDLIST Chunk_LinkestList;
	LINKEDLIST Free_Chunk_LinkedList;
}FREECHUNK;
typedef struct kInUseChunk{
	QWORD qwSize;
	QWORD qwMagic;
	LINKEDLIST Chunk_Linkestlist;
	LINKEDLIST padding;
}INUSECHUNK;
typedef struct kChunkManager{
	LINKEDLISTMANAGER Chunk_LinkedListManager;
	LINKEDLISTMANAGER Free_Chunk_LinkedListManager;
}CHUNKMANAGER;
#pragma pack(pop)
CHUNKMANAGER gs_stChunkManager;
void kInitializeDynamicMemory();
void* kMalloc(QWORD qwSize);
static FREECHUNK* kSelectFreeChunk(QWORD qwSize);
void kFree(void* pvAddress);
BOOL coalesce(FREECHUNK* pstCurrent,FREECHUNK* pstNext);
void Print_Free_Chunk(LINKEDLIST* pstLinkedList);
void Print_InUse_Chunk(LINKEDLIST* pstLinkedList);

void debug();

#endif /* __02_KERNEL64_SOURCE_DYNAMICMEMORY_H_ */
