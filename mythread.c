#include "mythread.h"
#include "queue.h"

#define THREAD_STACK 1024*8

//thread
Queue *readyQueue;
Queue *blockQueue;
Queue *deleteQueue;
Queue *reserveQueue;
Thread *currentThread;

ucontext_t processorContext;

//semaphore
SemList *semlist;

void movetoReady(Thread *thread){
	//if no elements in blockQueue
	if(blockQueue->front == NULL){
		//printf("error: fail to move thread to ready Queue\n");
		return;
	}
	//move to ready queue, delete thread in block queue
	enqueue(readyQueue, thread);
	if(removeFromQueue(blockQueue, thread) == 0)
		printf("error: cannot find thread in blockQueue\n");
}

void refreshWl(Queue *wl){
	//if no elements in thread's waiting list
	if(wl->front == NULL){
		//printf("error: fail to refresh waitlist -- no thread in waiting list\n");
		return;
	}
	QueueNode *qn = wl->front;

	//iterate through waiting queue, delete those that have finished
	while(qn != NULL){
		if((findQueue(readyQueue, qn->thread)==0) && (findQueue(blockQueue, qn->thread)==0)){
			removeFromQueue(wl, qn->thread);
				//printf("error: cannot find thread in waiting list\n");
		}
		qn = qn->next;
	}
}

void refreshBlockQueue(){
	//if no elements in blockQueue
	if(blockQueue->front == NULL){
		return;
	}
	
	//refresh waiting list of every threads in the block queue
	QueueNode *qn = blockQueue->front;
	while(qn != NULL){
		
		//first thread's wait list
		Queue *wl = qn->thread->waitlist;
		if(wl->front != NULL){
		//delete thread that are already terminated
			refreshWl(wl);
		}
		if(wl->front == NULL){
		//move thread to ready queue, delete thread in block queue
			movetoReady(qn->thread);
		}
		qn = qn->next;
	}
}

int aliveChild(){
	if(currentThread->child->front == NULL)
		return 0;
	else{
		QueueNode *qn = currentThread->child->front;
		while(qn != NULL){
		if((findQueue(readyQueue, qn->thread)==1) || (findQueue(blockQueue, qn->thread)==1)){
			return 1;
			}
		qn = qn->next;
		}
	}
	return 0;
}

void refreshReserveQueue(){
	if(reserveQueue->front == NULL)
		return;

	QueueNode *qn = reserveQueue->front;
	//iterate through waiting queue, delete those that have finished
	while(qn != NULL){
		if((findQueue(readyQueue, qn->thread)==0) && (findQueue(blockQueue, qn->thread)==0)){
			removeFromQueue(reserveQueue, qn->thread);
				//printf("error: cannot find thread in waiting list\n");
		}
		qn = qn->next;
	}
}

void refreshDeleteQueue(){
	Thread *thread;
	while(deleteQueue->front != NULL){
		Thread *thread = dequeue(deleteQueue);
		free((thread->ctxt).uc_stack.ss_sp);
		thread->child = NULL;
		free(thread->child);
		thread = NULL;
		free(thread);
	}
}

void MyThreadInit (void(*start_funct)(void *), void *args){
	//init queues
	readyQueue = (Queue *)malloc(sizeof(Queue));
	createQueue(readyQueue);
	blockQueue = (Queue *)malloc(sizeof(Queue));
	createQueue(blockQueue);
	deleteQueue = (Queue *)malloc(sizeof(Queue));
	createQueue(deleteQueue);
	reserveQueue = (Queue *)malloc(sizeof(Queue));
	createQueue(reserveQueue);

	//construct initial thread
	Thread *initThread = (Thread *)malloc(sizeof(Thread));
	//initThread -> parent = NULL;
	initThread -> child = (Queue *)malloc(sizeof(Queue));
	createQueue(initThread -> child);
	initThread -> waitlist = (Queue *)malloc(sizeof(Queue));
	createQueue(initThread -> waitlist);

	//construct context
	ucontext_t ctxt;
	getcontext(&ctxt);
	ctxt.uc_link = &processorContext;
  	(ctxt.uc_stack).ss_sp = malloc(THREAD_STACK);
	(ctxt.uc_stack).ss_size = THREAD_STACK;
	(ctxt.uc_stack).ss_flags = 0;
	makecontext(&ctxt, (void (*)()) start_funct, 1, args);
	//run the initial thread
	initThread->ctxt = ctxt;
	currentThread = initThread;
	swapcontext(&processorContext, &(currentThread->ctxt));
	//search for the next thread
	refreshBlockQueue();
	while(readyQueue->length > 0){
		currentThread = dequeue(readyQueue);
		swapcontext(&processorContext, &(currentThread->ctxt));
		refreshBlockQueue();
		refreshDeleteQueue();
	}
	return;
}


void MyThreadExit(void){	
	refreshReserveQueue();
	if(aliveChild()){
		enqueue(reserveQueue, currentThread);
		swapcontext(&(currentThread->ctxt),&processorContext);
	}
	else{
		enqueue(deleteQueue, currentThread);
		swapcontext(&(currentThread->ctxt),&processorContext);
	}
}

MyThread MyThreadCreate(void(*start_funct)(void *), void *args){
	//if no function is involved
	if(start_funct == NULL){
		//printf("error: not valid function\n");
		return NULL;
	}
	//construct thread
	Thread *thread = (Thread *)malloc(sizeof(Thread));
	//thread -> parent = currentThread;
	thread -> child = (Queue *)malloc(sizeof(Queue));
	createQueue(thread -> child);
	thread -> waitlist = (Queue *)malloc(sizeof(Queue));
	createQueue(thread -> waitlist);
	//create context
	ucontext_t ctxt;
	getcontext(&ctxt);
	ctxt.uc_link = &processorContext;
  	(ctxt.uc_stack).ss_sp = malloc(THREAD_STACK);
	(ctxt.uc_stack).ss_size = THREAD_STACK;
	(ctxt.uc_stack).ss_flags = 0;
	makecontext(&ctxt, (void (*)()) start_funct, 1, args);
	thread -> ctxt = ctxt;
	//add thread on queues
	enqueue(readyQueue, thread);
	enqueue(currentThread->child, thread);
	return thread;
}

void MyThreadYield(void) {
	if(readyQueue->length == 0)
		return;
	else{
		enqueue(readyQueue, currentThread);
		swapcontext(&(currentThread->ctxt), &processorContext);
	}
}

int MyThreadJoin(MyThread thread){
	//check if thread is immediate child
	Thread *temp = (Thread *)thread;
	if(findQueue(currentThread->child, thread) == 0){
		//printf("no immediate child\n");
		return -1;
	}

	//iterate through block queue and ready queue
	Thread *ready = returnQueue(readyQueue, temp);
	Thread *block = returnQueue(blockQueue, temp);

	//if thread not found, return directly
	if(ready == NULL && block == NULL){
		//printf("child already terminated\n");
		return 0;
	}

	//if thread found, add on waitlist, place current thread on block queue, 
	if(ready != NULL || block != NULL){
		enqueue(currentThread->waitlist, temp);
		enqueue(blockQueue, currentThread);
		swapcontext(&(currentThread->ctxt), &(processorContext));
		return 0;
	}
}

void MyThreadJoinAll(void) {
	//refresh child queue
	Queue *childQueue = currentThread->child;
	Queue *wl = currentThread->waitlist;
	if(childQueue->front == NULL){
		//printf("exmpty child queue\n");
		return;
	}

	//printf("lala\n");
	dupQueue(childQueue, wl);

	refreshWl(wl);
	if(wl->front == NULL){
		//printf("no active child\n");
		return;
	}
	//place current thread on block queue
	enqueue(blockQueue, currentThread);
	//swap to unix process
	swapcontext(&(currentThread->ctxt), &(processorContext));
}

MySemaphore MySemaphoreInit(int initialValue){
	//confirm if the input is not negative
	if(initialValue < 0){
		//printf("eligle initial value for semaphore\n");
		return NULL;
	}

	if(semlist == NULL){
		semlist = (SemList *)malloc(sizeof(SemList));
		semlist->head = NULL;
	}

	//create structure
	Semaphore *semaphore = (Semaphore *)malloc(sizeof(Semaphore));
	semaphore->value = initialValue;
	semaphore->blockQueue = (Queue *)malloc(sizeof(Queue));
	createQueue(semaphore->blockQueue);

	//add to the semlist
	insertSem(semlist, semaphore);

	//return
	return semaphore;
}

void MySemaphoreSignal(MySemaphore sem){
	Semaphore *semaphore = sem;	
	if(semlist == NULL){
		//printf("error: no sem initialized\n");
		return;
	}
	if(ifSem(semlist, semaphore) == 0){
		//printf("error: can't signal\n");
		return;
	}	
	//value ++
	semaphore->value ++;

	//if has some other thread waiting
	if(semaphore->value <= 0){
		if(semaphore->blockQueue->length >0)
			enqueue(readyQueue, dequeue(semaphore->blockQueue));
		else
			return;
	}
}

void MySemaphoreWait(MySemaphore sem){
	Semaphore *semaphore = sem;	
	if(semlist == NULL){
		//printf("error: no sem initialized\n");
		return;
	}
	if(ifSem(semlist, semaphore) == 0){
		//printf("error: can't wait\n");
		return;
	}

	//value --
	//printf("%d",semaphore->value);
	semaphore->value --;
			//printf("test\n");
	//if not enough resources
	if(semaphore->value < 0){
		//printf("test\n");
		//insert into the sem blocked queue
		enqueue(semaphore->blockQueue, currentThread);
		//printf("wait\n");
		//return to unix process
		swapcontext(&(currentThread->ctxt),&(processorContext));
	}
}

int MySemaphoreDestroy(MySemaphore sem){
	Semaphore *semaphore = sem;	
	if(semlist == NULL){
		//printf("error: no sem initialized\n");
		return -1;
	}

	if(semaphore->blockQueue->front != NULL){
		//printf("still have thread blocked in sem\n");
		return -1;
	}

	if(semaphore->blockQueue->front == NULL) {
		
		removeSem(semlist, semaphore);
		free(semaphore->blockQueue);
		semaphore = NULL;
		return 0;
	}
	
	return -1;
}


















