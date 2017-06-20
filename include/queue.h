
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>

typedef struct queue_t queue_t;

queue_t *queueAlloc();
void queueFree(queue_t *q);

bool queueEmpty(queue_t *q);
unsigned int queueSize(queue_t *q);
void *queueFront(queue_t *q);
void queuePop(queue_t *q);
void queuePushBack(queue_t *q, void *item);

#endif
