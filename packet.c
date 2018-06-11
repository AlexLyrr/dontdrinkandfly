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
				#ifdef JOYSTICK_ENABLE
				if (state.currentMode == 0 && (state.controlLift != 0 || state.controlRoll != 90 || state.controlPitch != 90 || state.controlYaw != 90)){
					break;
				}
				else{
					parsePacketSetMode();
					break;
				}
				#endif
				#ifndef JOYSTICK_ENABLE
					parsePacketSetMode();
					break;
				#endif
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
	state.controlLiftUser = state.currentPacket[9] << 8 | state.currentPacket[10];
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
	state.p1 = state.currentPacket[5] << 8 | state.currentPacket[6];
	state.p2 = state.currentPacket[7] << 8 | state.currentPacket[8];
	state.pYaw = state.currentPacket[9] << 8 | state.currentPacket[10];
	state.pLift = state.currentPacket[12];
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
	#ifdef APPLICATION_TIMINGS
	uint32_t avg = 0;
	if (state.timeLoopCount > 0) { // No div0
		avg = state.timeLoopTotal / state.timeLoopCount;
	}
	writePacket(14, 1,
		(state.timeLoopMax >> 24) & 0xFF, (state.timeLoopMax >> 16) & 0xFF, (state.timeLoopMax >> 8) & 0xFF, state.timeLoopMax & 0xFF,
		(avg >> 24) & 0xFF, (avg >> 16) & 0xFF, (avg >> 8) & 0xFF, avg & 0xFF);

	state.timeLoopTotal = 0;
	state.timeLoopCount = 0;
	state.timeLoopMax = 0;
	#endif
}

void writePing(uint32_t clock) {
	writePacket(12, 0,0,0,0, (clock >> 24) & 0xFF, (clock >> 16) & 0xFF, (clock >> 8) & 0xFF, clock & 0xFF, 0);
}

void writePong(uint32_t clock) {
	writePacket(13, 0,0,0,0, (clock >> 24) & 0xFF, (clock >> 16) & 0xFF, (clock >> 8) & 0xFF, clock & 0xFF, 0);
}

void writeSensorValues() {
	if (state.calibrated) {
		writePacket(15,
			((phi - state.calibratePhiOffset) >> 8) & 0xFF, (phi - state.calibratePhiOffset) & 0xFF,
			((theta - state.calibrateThetaOffset) >> 8) & 0xFF, (theta - state.calibrateThetaOffset) & 0xFF,
			((psi - state.calibratePsiOffset) >> 8) & 0xFF, (psi - state.calibratePsiOffset) & 0xFF,
			0, 0, 0
		);
		writePacket(16,
			((sp - state.calibrateSpOffset) >> 8) & 0xFF, (sp - state.calibrateSpOffset) & 0xFF,
			((sq - state.calibrateSqOffset) >> 8) & 0xFF, (sq - state.calibrateSqOffset) & 0xFF,
			((sr - state.calibrateSrOffset) >> 8) & 0xFF, (sr - state.calibrateSrOffset) & 0xFF,
			0, 0, 0
		);
	} else {
		writePacket(15,
			(phi >> 8) & 0xFF, phi & 0xFF,
			(theta >> 8) & 0xFF, theta & 0xFF,
			(psi >> 8) & 0xFF, psi & 0xFF,
			0, 0, 0
		);
		writePacket(16,
			(sp >> 8) & 0xFF, sp & 0xFF,
			(sq >> 8) & 0xFF, sq & 0xFF,
			(sr >> 8) & 0xFF, sr & 0xFF,
			0, 0, 0
		);
	}
}

void writeOffsetValues() {
	writePacket(15,
		(state.calibratePhiOffset >> 8) & 0xFF, state.calibratePhiOffset & 0xFF,
		(state.calibrateThetaOffset >> 8) & 0xFF, state.calibrateThetaOffset & 0xFF,
		(state.calibratePsiOffset >> 8) & 0xFF, state.calibratePsiOffset & 0xFF,
		0, 0, 0
	);
	writePacket(16,
		(state.calibrateSpOffset >> 8) & 0xFF, state.calibrateSpOffset & 0xFF,
		(state.calibrateSqOffset >> 8) & 0xFF, state.calibrateSqOffset & 0xFF,
		(state.calibrateSrOffset >> 8) & 0xFF, state.calibrateSrOffset & 0xFF,
		0, 0, 0
	);
}

void writeRawValues(uint16_t sp, uint16_t sq, uint16_t sr, uint16_t sax, uint16_t say, uint16_t saz) {
	writePacket(17,
		(sp >> 8) & 0xFF, sp & 0xFF,
		(sq >> 8) & 0xFF, sq & 0xFF,
		(sr >> 8) & 0xFF, sr & 0xFF,
		0, 0, 0
	);
	writePacket(18,
		(sax >> 8) & 0xFF, sax & 0xFF,
		(say >> 8) & 0xFF, say & 0xFF,
		(saz >> 8) & 0xFF, saz & 0xFF,
		0, 0, 0
	);
}
