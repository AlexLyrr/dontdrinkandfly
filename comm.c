#include "in4073.h"

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
	if ((state.currentPacket[1] & 0x80) == 0x80) {
		onAbort();
		return;
	}
	state.controlRoll = state.currentPacket[2];
	state.controlPitch = state.currentPacket[3];
	state.controlYaw = state.currentPacket[4];
	state.controlLift = state.currentPacket[5];
	state.controlChanged = true;
}

/*
 *
 * @author Joseph Verburg
 */
void parsePacketSetMode() {
	uint8_t mode = state.currentPacket[1];
	// TODO: processing here ?
	state.nextMode = mode;
}

/*
 *
 * @author Joseph Verburg
 */
void writeDroneStatus() {
	writeByte(0x02);
	writeByte(0x0F & state.currentMode);

	// Battery
	writeByte((uint8_t) (bat_volt >> 8));
	// Roll / phi
	writeByte((phi >> 8));
	// Pitch / theta
	writeByte((theta >> 8));
	// Height
	writeByte(0x00);
	// Time ?
	writeByte(0x00);
	writeByte(appClock >> 8);
	writeByte(appClock & 0xFF);

	writeByte(0x00);
}

/*
 *
 * @author Joseph Verburg
 */
void writeError(char errorCode) {
	writeByte(0x07);
	writeByte(errorCode);

	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);

	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
}