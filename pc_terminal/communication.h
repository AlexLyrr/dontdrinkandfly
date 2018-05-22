
#include <stdint.h>
#include <stdbool.h>
#include "./rs232.h"

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#define COMMUNICATION_MIN_DELAY_US (11 * 1000)
#define COMMUNICATION_PING_INTERVAL_US (1000 * 1000)

typedef struct {
	uint16_t fcs;
	uint8_t payload[10];
	uint8_t crc;
} SRPacket;

SRPacket sPacketBuffer[65535];
bool receivedACK[65535];
uint16_t lastPacketNum;

typedef struct {
	uint64_t lastPacketSent;
	uint64_t lastPacketReceived;
	uint64_t lastStatusUpdate;
	uint64_t lastPingSent;

	bool sendPing;

} CommunicationState;

void initializeCommunication();

void writeSetMode(uint8_t mode);
void writeSetControl(bool abort, uint8_t roll, uint8_t pitch, uint8_t yaw, uint16_t lift);
void writeSetP(uint16_t roll, uint16_t pitch, uint16_t yaw);

void writePing(uint64_t clk);
void writePong(uint64_t clk);

void writePacket(
    uint8_t b0, uint8_t b1,
    uint8_t b2, uint8_t b3,
    uint8_t b4, uint8_t b5,
    uint8_t b6, uint8_t b7,
    uint8_t b8, uint8_t b9);

CommunicationState communication;

#endif