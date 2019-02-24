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
	void* Begin_LinkedList; //LINKEDLIST�� ù��° ��带 ����Ű�� ������
	void* End_LinkedList;   //LINKEDLIST�� ������ ��带 ����Ű�� ������
	QWORD Total_Node_Count;	//���� ����ִ� ����� ���� ī��Ų��.
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
//Random access�� ��带 �߰�/����
BOOL insert(LINKEDLISTMANAGER* LinkedListManager, void* Next_LinkedList, QWORD Node_Position);
void* erase(LINKEDLISTMANAGER* LinkedListManager,QWORD Node_Position);
//���� �տ� ��� �߰�/����
BOOL push_front(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList);
void* pop_front(LINKEDLISTMANAGER* LinkedListManager);
//���� �ڿ� ��� �߰�/����
BOOL push_back(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList);
void* pop_back(LINKEDLISTMANAGER* LinkedListManager);
//linkedlist �ڷ� ���� �Լ�
void* get_next_node(LINKEDLIST* LinkedList);
void* get_prev_node(LINKEDLIST* LinkedList);
QWORD get_node_id(LINKEDLIST* LinkedList);
//debug
void Inconsistency_Found();
void Print_LinkedList();
#endif /* LINKEDLIST_H_ */
