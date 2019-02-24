/*
 * LinkedList.h
 *
 *  Created on: 2019. 2. 22.
 *      Author: DEWH
 */

#ifndef __LINKEDLIST_H_
#define __LINKEDLIST_H_
#include "Types.h"
#pragma pack(push, 1)
typedef struct stLinkedList{
	void* Next_LinkedList;
	void* Prev_LinkedList;
	QWORD Node_ID;
}LINKEDLIST;
typedef struct stLinkedListManager{
	void* Begin_LinkedList; //LINKEDLIST의 첫번째 노드를 가르키는 포인터
	void* End_LinkedList;   //LINKEDLIST의 마지막 노드를 가르키는 포인터
	QWORD Total_Node_Count;	//현재 들어있는 노드의 수를 카르킨다.
	QWORD Max_Count;
}LINKEDLISTMANAGER;
#pragma pack(pop)
//initilize function
void InitializeLinkedList(LINKEDLIST* LinkedList);
void InitializeLinkedListManger(LINKEDLISTMANAGER* LinkedListManager);
void SetLinkedList(LINKEDLIST* LinkedList,LINKEDLIST* Next_List,LINKEDLIST* Prev_list);
QWORD count(LINKEDLISTMANAGER* LinkedListManager);
int find(LINKEDLISTMANAGER* LinkedListManager, QWORD Node_ID);
void* front(LINKEDLISTMANAGER* LinkedListManager);
void* back(LINKEDLISTMANAGER* LinkedListManager);
void clear(LINKEDLISTMANAGER* LinkedListManager);
//Random access로 노드를 추가/제거
BOOL insert(LINKEDLISTMANAGER* LinkedListManager, void* Next_LinkedList, QWORD Node_Position);
void* erase(LINKEDLISTMANAGER* LinkedListManager,QWORD Node_Position);
//제일 앞에 노드 추가/제거
BOOL push_front(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList);
void* pop_front(LINKEDLISTMANAGER* LinkedListManager);
//제일 뒤에 노드 추가/제거
BOOL push_back(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList);
void* pop_back(LINKEDLISTMANAGER* LinkedListManager);
//linkedlist 자료 접근 함수
void* get_next_node(LINKEDLIST* LinkedList);
void* get_prev_node(LINKEDLIST* LinkedList);
QWORD get_node_id(LINKEDLIST* LinkedList);
//debug
void Inconsistency_Found();
void Print_LinkedList();
#endif /* LINKEDLIST_H_ */
