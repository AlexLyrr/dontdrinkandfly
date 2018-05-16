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
			case 12:
				parsePacketPing();
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
	state.controlRollUser = state.currentPacket[6];
	state.controlPitch = state.currentPacket[7];
	state.controlPitchUser = state.currentPacket[7];
	state.controlYaw = state.currentPacket[8];
	state.controlYawUser = state.currentPacket[8];
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

void parsePacketPing() {
	writePacket(13, state.currentPacket[5], state.currentPacket[6], state.currentPacket[7], state.currentPacket[8],
	state.currentPacket[9], state.currentPacket[10], state.currentPacket[11], state.currentPacket[12],
	0x00);
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
		(uint8_t) (bat_volt),
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

void writeTimings() {
	// writePacket(14, 1, 255, 255, 255, 251, 250, 249, 248, 247);
	writePacket(14, 1, 
		(state.timeLoopMax >> 24) & 0xFF, (state.timeLoopMax >> 16) & 0xFF, (state.timeLoopMax >> 8) & 0xFF, state.timeLoopMax & 0xFF, 
		(state.timeLoop >> 24) & 0xFF, (state.timeLoop >> 16) & 0xFF, (state.timeLoop >> 8) & 0xFF, state.timeLoop & 0xFF);

	writePacket(14, 2, 
		(state.timeLoopPacketMax >> 24) & 0xFF, (state.timeLoopPacketMax >> 16) & 0xFF, (state.timeLoopPacketMax >> 8) & 0xFF, state.timeLoopPacketMax & 0xFF, 
		(state.timeLoopPacket >> 24) & 0xFF, (state.timeLoopPacket >> 16) & 0xFF, (state.timeLoopPacket >> 8) & 0xFF, state.timeLoopPacket & 0xFF);

	writePacket(14, 3, 
		(state.timeLoopAppMax >> 24) & 0xFF, (state.timeLoopAppMax >> 16) & 0xFF, (state.timeLoopAppMax >> 8) & 0xFF, state.timeLoopAppMax & 0xFF, 
		(state.timeLoopApp >> 24) & 0xFF, (state.timeLoopApp >> 16) & 0xFF, (state.timeLoopApp >> 8) & 0xFF, state.timeLoopApp & 0xFF);

	writePacket(14, 4, 
		(state.timeLoopControlMax >> 24) & 0xFF, (state.timeLoopControlMax >> 16) & 0xFF, (state.timeLoopControlMax >> 8) & 0xFF, state.timeLoopControlMax & 0xFF, 
		(state.timeLoopControl >> 24) & 0xFF, (state.timeLoopControl >> 16) & 0xFF, (state.timeLoopControl >> 8) & 0xFF, state.timeLoopControl & 0xFF);

	writePacket(14, 5, 
		(state.timeLoopSensorMax >> 24) & 0xFF, (state.timeLoopSensorMax >> 16) & 0xFF, (state.timeLoopSensorMax >> 8) & 0xFF, state.timeLoopSensorMax & 0xFF, 
		(state.timeLoopSensor >> 24) & 0xFF, (state.timeLoopSensor >> 16) & 0xFF, (state.timeLoopSensor >> 8) & 0xFF, state.timeLoopSensor & 0xFF);


	state.timeLoop = 0;
	state.timeLoopMax = 0;
	state.timeLoopPacket = 0;
	state.timeLoopPacketMax = 0;
	state.timeLoopApp = 0;
	state.timeLoopAppMax = 0;
	state.timeLoopControl = 0;
	state.timeLoopControlMax = 0;
	state.timeLoopSensor = 0;
	state.timeLoopSensorMax = 0;
}

void writePing(uint32_t clock) {
	writePacket(12, 0,0,0,0, (clock >> 24) & 0xFF, (clock >> 16) & 0xFF, (clock >> 8) & 0xFF, clock & 0xFF, 0);
}

void writePong(uint32_t clock) {
	writePacket(13, 0,0,0,0, (clock >> 24) & 0xFF, (clock >> 16) & 0xFF, (clock >> 8) & 0xFF, clock & 0xFF, 0);
}