#include "in4073.h"




void packetComponentLoop() {
	if (state.hasPacket) {
		state.hasPacket = false;
		switch(state.currentPacket[4]) {
			case 0x01:
				parsePacketInit();
				break;
			case 0x03:
				parsePacketSetControl();
				break;
			case 0x05:
				parsePacketSetMode();
				break;
			case 0x09:
				parsePacketSetP();
				break;
			default:
				state.packetError = 1;
				break;
		}
	}
}

/*
 *
 * @author Joseph Verburg
 */
void parsePacketInit() {
	state.sendStatus = true;
}

/*
 *
 * @author Joseph Verburg
 */
void parsePacketSetControl() {
	if (state.currentPacket[5] > 0) {
		onAbort();
		return;
	}
	state.controlRoll = state.currentPacket[6];
	state.controlPitch = state.currentPacket[7];
	state.controlYaw = state.currentPacket[8];
	state.controlLift = state.currentPacket[9] << 8 | state.currentPacket[10];
	state.controlChanged = true;
}

/*
 *
 * @author Joseph Verburg
 */
void parsePacketSetMode() {
	uint8_t mode = state.currentPacket[5];
	state.nextMode = mode;
}
/*
 *
 * @author Joseph Verburg
 */
void parsePacketSetP() {
	state.pRoll = state.currentPacket[5] << 8 | state.currentPacket[6];
	state.pPitch = state.currentPacket[7] << 8 | state.currentPacket[8];
	state.pYaw = state.currentPacket[9] << 8 | state.currentPacket[10];
	state.pChanged = true;
}

/*
 *
 * @author Joseph Verburg
 */
void writeDroneStatus() {
	writePacket(
		0x02,
		0x0F & state.currentMode,

		// Battery
		(uint8_t) (bat_volt >> 8),
		// Roll / phi
		(phi >> 8),
		// Pitch / theta
		(theta >> 8),
		// Height
		0x00,
		// Time ?
		0x00,
		appClock >> 8,
		appClock & 0xFF,

		0x00
	);
}

/*
 *
 * @author Joseph Verburg
 */
void writeError(uint8_t errorCode) {
	writePacket(
		0x07,
		errorCode,

		0x00,
		0x00,
		0x00,

		0x00,
		0x00,
		0x00,
		0x00,
		0x00
	);
}

void writeMotorStatus() {
	writePacket(
		0x0a,

		motor[0] >> 8,
		motor[0] & 0xFF,

		motor[1] >> 8,
		motor[1] & 0xFF,

		motor[2] >> 8,
		motor[2] & 0xFF,

		motor[3] >> 8,
		motor[3] & 0xFF,

		0x00
	);
}

void writeAck(uint16_t packetNumber) {
	writePacket(
		0x0b,

		packetNumber >> 8,
		packetNumber & 0xFF,
		0x00,
		0x00,

		0x00,
		0x00,
		0x00,
		0x00,
		0x00
	);
}
