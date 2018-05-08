#include <stdint.h>

#ifndef PCQUEUE_H_
#define PCQUEUE_H_
// Queue
#define QUEUE_SIZE 256
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint16_t first, last;
	uint16_t count;
} queue;
void init_queue(queue *q);
void enqueue(queue *q, char x);
char dequeue(queue *q);
char queuePeek(queue *q, uint16_t offset);

queue pcReQueue;

#endif
