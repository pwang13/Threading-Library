#include <stdio.h>
#include <malloc.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stddef.h>

struct Queue;
struct QueueNode;
struct Thread;

typedef struct Thread{
	ucontext_t ctxt;
	struct Queue *child;
	struct Queue *waitlist;
}Thread;

typedef struct Queue{
	struct QueueNode *front;
	struct QueueNode *rear;
	int length;
} Queue;

typedef struct QueueNode{
	Thread *thread;
	struct QueueNode *next; 
} QueueNode;

typedef struct Semaphore{
	int value;
	Queue *blockQueue;
} Semaphore;

typedef struct ListNode {
	Semaphore *sem;
	struct ListNode *next;
} ListNode;

typedef struct SemaphoreList {
	ListNode *head;
	int length;
} SemList;		


void createQueue(Queue *q);
void enqueue(Queue *q, Thread *thread);
Thread *dequeue(Queue *q);
int findQueue(Queue *q, Thread *thread);
Thread *returnQueue(Queue *q, Thread *thread);
int removeFromQueue(Queue *q, Thread *thread);
void dupQueue(Queue *q, Queue *qDup);












