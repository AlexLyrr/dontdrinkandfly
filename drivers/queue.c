/*------------------------------------------------------------------
 *  queue.c -- some queue implementation stolen from the interwebs
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"

void init_queue(queue *q){
	
	q->first = 0;
	q->last = QUEUE_SIZE - 1;
	q->count = 0;
}

void enqueue(queue *q,uint8_t x){

	q->last = (q->last + 1) % QUEUE_SIZE;
	q->Data[ q->last ] = x;
	q->count += 1;
}

uint8_t dequeue(queue *q){

	uint8_t x = q->Data[ q->first ];
	q->first = (q->first + 1) % QUEUE_SIZE;
	q->count -= 1;
	return x;
}

uint8_t queuePeek(queue *q, uint16_t offset) {
	offset = (q->first + offset) % QUEUE_SIZE;
	return q->Data[ offset ];
}