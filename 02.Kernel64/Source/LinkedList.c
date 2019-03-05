/*
 * LinkedList.c
 *
 *  Created on: 2019. 2. 22.
 *      Author: DEWH
 */


#include "LinkedList.h"
#include "Task.h"
#include "Console.h"
void InitializeLinkedList(LINKEDLIST* LinkedList){
	LinkedList->Next_LinkedList = NULL;
	LinkedList->Prev_LinkedList = NULL;
	LinkedList->Node_ID = 0UL;
}
void InitializeLinkedListManger(LINKEDLISTMANAGER* LinkedListManager){
	LinkedListManager->Begin_LinkedList = NULL;
	LinkedListManager->End_LinkedList = NULL;
	LinkedListManager->Total_Node_Count = 0UL;
	LinkedListManager->Max_Count = LINKEDLIST_MAX;
}
void SetLinkedList(LINKEDLIST* LinkedList,LINKEDLIST* Next_List,LINKEDLIST* Prev_list){
	LinkedList->Next_LinkedList = Next_List;
	LinkedList->Prev_LinkedList = Prev_list;
}
QWORD count(LINKEDLISTMANAGER* LinkedListManager){
	return LinkedListManager->Total_Node_Count;
}
int find(LINKEDLISTMANAGER* LinkedListManager, QWORD Node_ID){
	LINKEDLIST* Iterator;
	QWORD Check_value;
	int	iIndex=0;
	Check_value = count(LinkedListManager);
	for(Iterator = (LINKEDLIST*)front(LinkedListManager); Iterator!=NULL;\
	Iterator=Iterator->Next_LinkedList){
		if(Iterator->Node_ID == Node_ID){
			return iIndex;
		}
		Check_value--;
		iIndex++;
	}
	if(Check_value!=0UL){
		Inconsistency_Found(LinkedListManager);
	}
	Inconsistency_Found(LinkedListManager);
	return -1;
}
void* front(LINKEDLISTMANAGER* LinkedListManager){
	return LinkedListManager->Begin_LinkedList;
}
void* back(LINKEDLISTMANAGER* LinkedListManager){
	return LinkedListManager->End_LinkedList;
}
void clear(LINKEDLISTMANAGER* LinkedListManager){

}
//Random access로 노드를 추가/제거
BOOL insert(LINKEDLISTMANAGER* LinkedListManager, void* Next_LinkedList, QWORD Node_Position){
	LINKEDLIST* next_node, * prev_node, * current_node;
	QWORD node_count;
	QWORD index;
	node_count = count(LinkedListManager);
	if(node_count < Node_Position || node_count == LinkedListManager->Max_Count){
		return FALSE;
	}
	/*
	 * 이전에 current_node 즉 empty일때 경우를 처리해야한다.
	 */
	current_node = Next_LinkedList;
	if(node_count == 0){
		//empty처리
		SetLinkedList(current_node, NULL, NULL);

		LinkedListManager->Begin_LinkedList = current_node;
		LinkedListManager->End_LinkedList = current_node;
		LinkedListManager->Total_Node_Count = node_count + 1;
		return TRUE;
	}else{
		next_node = front(LinkedListManager);
		for(index=0UL;index<Node_Position;index++){
			next_node = next_node->Next_LinkedList;
		}
		if(next_node == NULL){
			//이 경우는 push_back에 해당하는 경우이다.
			prev_node = (LINKEDLIST*)back(LinkedListManager);
			SetLinkedList(current_node, NULL, prev_node);
			SetLinkedList(prev_node, current_node, get_prev_node(prev_node));

			LinkedListManager->End_LinkedList = current_node;
			LinkedListManager->Total_Node_Count = node_count + 1;
			return TRUE;
		}else{
			prev_node = get_prev_node(next_node);
			if(prev_node == NULL){
			//이 경우는 push_front에 해당하는 경우이다.
				SetLinkedList(current_node, next_node, NULL);
				SetLinkedList(next_node, get_next_node(next_node), current_node);

				LinkedListManager->Begin_LinkedList = current_node;
				LinkedListManager->Total_Node_Count = node_count + 1;
				return TRUE;
			}else{
			//앞위에 노드가 존재하는 경우
				SetLinkedList(prev_node, current_node, get_prev_node(prev_node));
				SetLinkedList(current_node, next_node, prev_node);
				SetLinkedList(next_node, get_next_node(next_node), current_node);

				LinkedListManager->Total_Node_Count = node_count + 1;
				return TRUE;
			}
		}
	}
}
void* erase(LINKEDLISTMANAGER* LinkedListManager,QWORD Node_Position){
	LINKEDLIST* next_node, * prev_node, * current_node;
	QWORD node_count;
	QWORD index;
	node_count = count(LinkedListManager);
	if(node_count <= Node_Position){//empty인 경우도 여기서 처리된다.
		return NULL;
	}
	current_node = front(LinkedListManager);
	for(index=0UL;index<Node_Position;index++){
		current_node = current_node->Next_LinkedList;
	}
	next_node = get_next_node(current_node);
	prev_node = get_prev_node(current_node);
	if(next_node == NULL && prev_node == NULL){
		//앞 뒤 노드가 비어있는 경우, 즉 노드 갯수가 1개일때이다.
		if(node_count != 1){
			Inconsistency_Found(LinkedListManager);
			return NULL;
		}
		SetLinkedList(current_node, NULL, NULL);
		LinkedListManager->Begin_LinkedList = NULL;
		LinkedListManager->End_LinkedList = NULL;
		LinkedListManager->Total_Node_Count = 0;
		return current_node;
	}else if(next_node != NULL && prev_node == NULL){
		//뒤 노드만 있는 경우. pop front와 같다. < x o o ~ >
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(next_node, get_next_node(next_node), NULL);

		LinkedListManager->Begin_LinkedList = next_node;
		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}else if(next_node == NULL && prev_node != NULL){
		//앞 노드만 있는 경우. pop back과 같다. <~ o o x>
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(prev_node, NULL, get_prev_node(prev_node));

		LinkedListManager->End_LinkedList = prev_node;
		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}else{
		SetLinkedList(prev_node, next_node, get_prev_node(prev_node));
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(next_node, get_next_node(next_node), prev_node);

		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}
}
void* erase_node(LINKEDLISTMANAGER* LinkedListManager,LINKEDLIST* pstCurrent_node){
	LINKEDLIST* next_node, * prev_node, * current_node;
	QWORD node_count;
	QWORD index;
	node_count = count(LinkedListManager);
	current_node = pstCurrent_node;
	next_node = get_next_node(current_node);
	prev_node = get_prev_node(current_node);
	if(next_node == NULL && prev_node == NULL){
		//앞 뒤 노드가 비어있는 경우, 즉 노드 갯수가 1개일때이다.
		if(node_count != 1){
			Inconsistency_Found(LinkedListManager);
			return NULL;
		}
		SetLinkedList(current_node, NULL, NULL);
		LinkedListManager->Begin_LinkedList = NULL;
		LinkedListManager->End_LinkedList = NULL;
		LinkedListManager->Total_Node_Count = 0;
		return current_node;
	}else if(next_node != NULL && prev_node == NULL){
		//뒤 노드만 있는 경우. pop front와 같다. < x o o ~ >
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(next_node, get_next_node(next_node), NULL);

		LinkedListManager->Begin_LinkedList = next_node;
		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}else if(next_node == NULL && prev_node != NULL){
		//앞 노드만 있는 경우. pop back과 같다. <~ o o x>
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(prev_node, NULL, get_prev_node(prev_node));

		LinkedListManager->End_LinkedList = prev_node;
		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}else{
		SetLinkedList(prev_node, next_node, get_prev_node(prev_node));
		SetLinkedList(current_node, NULL, NULL);
		SetLinkedList(next_node, get_next_node(next_node), prev_node);

		LinkedListManager->Total_Node_Count-=1;
		return current_node;
	}
}
BOOL insert_behind(LINKEDLISTMANAGER* LinkedListManager, void* Next_LinkedList, LINKEDLIST* Prev_LinkedList){
	LINKEDLIST* next_node, * prev_node, * current_node;
	QWORD node_count;
	QWORD index;
	if(Prev_LinkedList == NULL){
		return push_front(LinkedListManager, Next_LinkedList);
	}
	node_count = count(LinkedListManager);
	if( node_count == LinkedListManager->Max_Count){
		return FALSE;
	}
	/*
	 * 이전에 current_node 즉 empty일때 경우를 처리해야한다.
	 */
	current_node = Next_LinkedList;
	if(node_count == 0){
		//empty처리
		SetLinkedList(current_node, NULL, NULL);

		LinkedListManager->Begin_LinkedList = current_node;
		LinkedListManager->End_LinkedList = current_node;
		LinkedListManager->Total_Node_Count = node_count + 1;
		return TRUE;
	}else{
		next_node = get_next_node(Prev_LinkedList);
		if(next_node == NULL){
			//이 경우는 push_back에 해당하는 경우이다.
			prev_node = Prev_LinkedList;
			SetLinkedList(current_node, NULL, prev_node);
			SetLinkedList(prev_node, current_node, get_prev_node(prev_node));

			LinkedListManager->End_LinkedList = current_node;
			LinkedListManager->Total_Node_Count = node_count + 1;
			return TRUE;
		}else{
			prev_node = Prev_LinkedList;
			if(prev_node == NULL){
			//이 경우는 push_front에 해당하는 경우이다.
				SetLinkedList(current_node, next_node, NULL);
				SetLinkedList(next_node, get_next_node(next_node), current_node);

				LinkedListManager->Begin_LinkedList = current_node;
				LinkedListManager->Total_Node_Count = node_count + 1;
				return TRUE;
			}else{
			//앞위에 노드가 존재하는 경우
				SetLinkedList(prev_node, current_node, get_prev_node(prev_node));
				SetLinkedList(current_node, next_node, prev_node);
				SetLinkedList(next_node, get_next_node(next_node), current_node);

				LinkedListManager->Total_Node_Count = node_count + 1;
				return TRUE;
			}
		}
	}
}
//제일 앞에 노드 추가/제거
BOOL push_front(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList){
	return insert(LinkedListManager, Next_LinkedList, 0);
}
void* pop_front(LINKEDLISTMANAGER* LinkedListManager){
	//Print_LinkedList(LinkedListManager);
	return erase(LinkedListManager, 0);
	//Print_LinkedList(LinkedListManager);
}
//제일 뒤에 노드 추가/제거
BOOL push_back(LINKEDLISTMANAGER* LinkedListManager,void* Next_LinkedList){
	//Print_LinkedList(LinkedListManager);
	return insert(LinkedListManager, Next_LinkedList, count(LinkedListManager));
	//Print_LinkedList(LinkedListManager);
}
void* pop_back(LINKEDLISTMANAGER* LinkedListManager){
	return erase(LinkedListManager,count(LinkedListManager));
}
//linkedlist 자료 접근 함수
void* get_next_node(LINKEDLIST* LinkedList){
	return LinkedList->Next_LinkedList;
}
void* get_prev_node(LINKEDLIST* LinkedList){
	return LinkedList->Prev_LinkedList;
}
QWORD get_node_id(LINKEDLIST* LinkedList){
	return LinkedList->Node_ID;
}
//debug
void Inconsistency_Found(LINKEDLISTMANAGER* LinkedListManager){
	/*
	 * print()...
	 */
	Print_LinkedList(LinkedListManager,NULL);
}
void Print_LinkedList(LINKEDLISTMANAGER* LinkedListManager, void* Print_Structure){
	LINKEDLIST* Iterator;
	int iCount = 0;
	void (*Print_Inner)(LINKEDLIST* pstLinkedList);
	Print_Inner = Print_Structure;
	kPrintf("<LINKEDLISTMANAGER> Begin:%q\t End:%q\t Count:%d\n",\
			LinkedListManager->Begin_LinkedList,\
			LinkedListManager->End_LinkedList,\
			LinkedListManager->Total_Node_Count);
	kPrintf("<LINKEDLIST>\n");
	for(Iterator=(LINKEDLIST*)front(LinkedListManager);Iterator!=NULL;\
	Iterator=Iterator->Next_LinkedList){
		if(++iCount % 5 == 0){
		kPrintStringXY(0, CONSOLE_HEIGHT-1, "Press and key to continue...('q' is exit) : ");
			if(kGetCh()=='q'){
				kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");
				break;
			}
		}
		kPrintStringXY(0, CONSOLE_HEIGHT-1, "                                            ");
		kPrintf("\tAddress:%q\tNext:%q\tPrev:%q\tID:%q\n",\
				Iterator,Iterator->Next_LinkedList,Iterator->Prev_LinkedList,Iterator->Node_ID);
		if(Print_Inner != NULL)
			Print_Inner(Iterator);

	}
}

