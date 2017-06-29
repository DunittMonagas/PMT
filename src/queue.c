

#include <assert.h>
#include <stdlib.h>

#include <queue.h>


typedef struct queueNode{

	void *item;
	struct queueNode *next;

}queueNode;

typedef struct queue_t{

	queueNode *head;
	queueNode *tail;
	unsigned int size;
	bool (*comparisonFunction)(void*, void*);

}queue_t;

//Constructor
queue_t *queueAlloc(bool (*comp)(void*, void*)){

	queue_t *q= (queue_t*) malloc(sizeof(queue_t));

	q->size= 0;
	q->head= NULL;
	q->tail= NULL;
	q->comparisonFunction= comp;

	return q;

}

//Destructor
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

//Si comparisonFunction == NULL, entonces la cola será FIFO.
static void _queuePushBack(queue_t *q, void *item){

	queueNode *new= (queueNode*) malloc(sizeof(queueNode));

	new->next= NULL;
	new->item= item;

	if(queueSize(q))
		q->tail->next= new;
	else
		q->head= new;

	q->tail= new;
	q->size++;

}

void queuePushBack(queue_t *q, void *item){

	if(q->comparisonFunction == NULL){
		_queuePushBack(q, item);
		return;
	}

	queueNode *new= (queueNode*) malloc(sizeof(queueNode));

	new->next= NULL;
	new->item= item;

	if(q->head == NULL || q->comparisonFunction(q->head->item, new->item)){

		new->next= q->head;
		q->head= new;

	}else{

		queueNode *it= q->head;
		while(it->next != NULL && q->comparisonFunction(new->item, it->next->item))
			it= it->next;

		new->next= it->next;
		it->next= new;

	}

	q->size++;
	
}

//Establece la función de comparación.
void queueSetupComparisonFunction(queue_t *q, bool (*comp)(void*, void*)){
	q->comparisonFunction= comp;
}
