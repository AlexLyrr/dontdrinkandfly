#include <stdint.h>

#ifndef PCQUEUE_H_
#define PCQUEUE_H_
// Queue
#define PCQUEUE_SIZE 1024

typedef struct {
	uint8_t Data[PCQUEUE_SIZE];
	uint16_t first, last;
	uint16_t count;
} pcqueue;

void init_queuepc(pcqueue *q);
void enqueuepc(pcqueue *q, uint8_t x);
uint8_t dequeuepc(pcqueue *q);
uint8_t queuePeekpc(pcqueue *q, uint16_t offset);

pcqueue pcReQueue;

pcqueue ble_tx_queue;

#endif
