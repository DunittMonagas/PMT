
#include "queue.h"

typedef struct queueNode{

	void *item;
	queueNode *next;

}queueNode;

typedef struct queue_t{

	queueNode *head;
	queueNode *tail;
	unsigned int size;

}queue_t;

queue_t *queueAlloc(){

	queue_t *q= (queue_t*) malloc(sizeof(queue_t));

	q->size= 0;
	q->head= NULL;
	q->tail= NULL;

	return q;

}

void queueFree(queue_t *q){

	while(queueSize(q))
		queuePop(q);

	free(q);

}

bool queueEmpty(queue_t *q){
	return (q->size == 0);
}

unsigned int queueSize(queue_t *q){
	return q->size;
}

void *queueFront(queue_t *q){
	return q->head->item;
}

void queuePop(queue_t *q){

	assert(queueSize(q) > 0);

	queueNode *front;

	front= q->head;
	q->head= q->head->next;
	q->size--;

	free(front);

}

void queuePushBack(queue_t *q, void *item){

	queueNode *back= (queueNode*) malloc(sizeof(queueNode));

	back->next= NULL;
	back->item= item;

	if(queueSize(q))
		q->tail->next= back;
	else
		q->head= back;

	q->tail= back;
	q->size++;
	
}