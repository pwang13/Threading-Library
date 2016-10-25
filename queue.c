
#include "queue.h"

//queue
void createQueue(Queue *q) {
	//q = (Queue *)malloc(sizeof(Queue));
	q -> front = NULL;
	q -> rear = NULL;
	q -> length = 0;
}

void enqueue(Queue *q, Thread *thread) {
	QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
	node -> thread = thread;

	node -> next = NULL;
	if(q -> length == 0) {

		q -> front = node;
	}
	else {

		q -> rear -> next = node;
	}	
	q -> rear = node;
	q -> length ++;
	//printf("%d\n",q->length);
}

Thread *dequeue(Queue *q) {
	Thread *thread = (Thread *)malloc(sizeof(Thread));
	//printf("%d\n",q->length);
	if(q -> length == 0){
		thread = NULL;
	}
	else{
		thread = q->front->thread;
		if(q -> length == 1) {

			q->front = q->rear = NULL;
			q->length = 0;
		}
		else{
			q->front = q->front->next;
			q->length --;
		}
	}
	return thread;
}



int findQueue(Queue *q, Thread *thread){
	if(q->front == NULL)
		return 0;
	QueueNode *qnode = q->front;
	while(qnode != NULL){
		if(qnode->thread == thread)
			return 1;
		qnode = qnode->next;
	}
	return 0;
}

Thread *returnQueue(Queue *q, Thread *thread){
	if(q->length == 0)
		return NULL;
	QueueNode *qn = q->front;
	while(qn != NULL){
		if(qn->thread == thread){
			return qn->thread;
		}
		qn = qn->next;
	}
	return NULL;
}

int removeFromQueue(Queue *q, Thread *thread) {
	QueueNode *p = q->front;
	QueueNode *r = q->front;
	while(p != NULL) {
		if(p->thread == thread) {
			if(q -> length == 1) {
			q->front = q->rear = NULL;
			q->length = 0;
			}
			else if(p == q->front){
				q->front = p->next;
				q->length --;
			}
			else if(p == q->rear) {
				q->rear = r;
				r->next = NULL;
				q->length --;
			}	
			else{
				r->next = p->next;
				q->length --;
			}
			free(p);

			return 1;
		}	
		r = p;
		p = p-> next;
	}	
	return 0;
}	


void dupQueue(Queue *q, Queue *qDup){

	QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
	node = q->front;
	while(node != NULL){
		enqueue(qDup, node->thread);
		node = node->next;
	}
}


//semaphore
int isListEmpty(SemList *list) {
	if(list->head == NULL)
		return 1;
	return 0;	
}	

void insertSem(SemList *list, Semaphore *sem) {
	ListNode *n = (ListNode *)malloc(sizeof(ListNode));
	n->sem = sem;
	n->next = (isListEmpty(list)) ? NULL : list->head->next;
	list->head = n;
	list->length ++;
}	

int removeSem(SemList *list, Semaphore *sem) {
	ListNode *p = list->head;
	ListNode *q = list->head;
	while(p != NULL) {
		if(p->sem == sem) {
			if(p == list->head)
				list->head = list->head->next;
			else
				q->next = p->next;
			free(p);
			list->length --;
			return 1;	
		}	
		q = p;
		p = p->next;
	}	
	return 0;
}	

int ifSem(SemList *list, Semaphore *sem) {
	if(list->head == NULL){
		return 0;
	}
	
	ListNode *p = list->head;
	while(p != NULL){
		if(p->sem == sem)
			return 1;
		p=p->next;
	}
	return 0;
}




