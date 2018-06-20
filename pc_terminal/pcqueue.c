/*------------------------------------------------------------------
 *  queue.c -- some queue implementation stolen from the interwebs
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */
#include "pcqueue.h"

void init_queuepc(pcqueue* q) {

	q->first = 0;
	q->last = PCQUEUE_SIZE - 1;
	q->count = 0;
}

void enqueuepc(pcqueue* q, uint8_t x) {

	q->last = (q->last + 1) % PCQUEUE_SIZE;
	q->Data[q->last] = x;
	q->count += 1;
}

uint8_t dequeuepc(pcqueue* q) {

	uint8_t x = q->Data[q->first];
	q->first = (q->first + 1) % PCQUEUE_SIZE;
	q->count -= 1;
	return x;
}

uint8_t queuePeekpc(pcqueue* q, uint16_t offset) {
	offset = (q->first + offset) % PCQUEUE_SIZE;
	return q->Data[offset];
}