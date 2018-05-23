/*------------------------------------------------------------------
 *  control.c -- here you can implement your control algorithm
 *		 and any motor clipping or whatever else
 *		 remember! motor input =  0-1000 : 125-250 us (OneShot125)
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
uint32_t prevTime = 0;
int16_t prevDisp = 0;
int16_t displacement;


/* 

Notes: 

Multipliers P1, P2 around 5, Roll/Pitch work for values between 0-15
Multiplier P around 4, Yaw work for values between 0-15

TBD: 

map range 90-180 to 0-15 or the other way around.
*/

void yawControl() {
	int32_t eps = (((int32_t) state.controlYawUser - 90) >> 3) - (((int32_t) sr - state.calibrateSrOffset) >> 7);
	int32_t yawValue = (state.pYaw * eps) + 90;
	if (yawValue > 180)
		yawValue = 180;
	if (yawValue < 0)
		yawValue = 0;
	state.controlYaw = (uint8_t) yawValue;
}

void pitchControl(){
	int32_t eps = (((int32_t) state.controlPitchUser - 90) >> 3) - ((theta - state.calibrateThetaOffset) >> 8); 
	int32_t pitchValue = (state.p1 * eps);
	int32_t eps2 = pitchValue - ((sq - state.calibrateSqOffset) >> 7);
	pitchValue = (state.p2 * eps2) + 90;
	if (pitchValue > 180)
		pitchValue = 180;
	if (pitchValue < 0)
		pitchValue = 0;
	state.controlPitch = (uint8_t) pitchValue;
}

void rollControl(){
	int32_t eps = (((int32_t) state.controlRollUser - 90) >> 3) - ((phi - state.calibratePhiOffset) >> 8);
	int32_t rollValue = (state.p1 * eps);
	int32_t eps2 = rollValue - ((sp - state.calibrateSpOffset) >> 7);
	rollValue = (state.p2 * eps2) + 90;
	if (rollValue > 180)
		rollValue = 180;
	if (rollValue < 0)
		rollValue = 0;
	state.controlRoll = (uint8_t) rollValue;
}

/*
void rollControl() {
	int32_t eps = ((int32_t) state.controlRollUser - 90) - (sp / 100);
	int32_t rollValue = (state.pRoll * eps) + 90;
	if (rollValue > 180)
		rollValue = 180;
	if (rollValue < 0)
		rollValue = 0;
	state.controlRoll = (uint8_t) rollValue;
}
*/


/*
void rollControl(){
	int32_t eps = ((int32_t) state.controlRollUser - 90) - ((phi - state.calibratePhiOffset) >> 8);
	int32_t rollValue = (state.pRoll * eps) + 90;
	if (rollValue > 180)
		rollValue = 180;
	if (rollValue < 0)
		rollValue = 0;
	state.controlRoll = (uint8_t) rollValue;
}
*/

/*
void pitchControl() {
	int32_t eps = ((int32_t) state.controlPitchUser - 90) - (sq / 100);
	int32_t pitchValue = (state.pPitch * eps) + 90;
	if (pitchValue > 180)
		pitchValue = 180;
	if (pitchValue < 0)
		pitchValue = 0;
	state.controlPitch = (uint8_t) pitchValue;
}
*/

/*
void pitchControl() {
	int32_t eps = ((int32_t) state.controlPitch - 90) * 100 - sp;
	state.controlPitch = (100 * eps)/100 + 90;
}

void pitchControl() {

	int32_t error = 0;
	int32_t spNew = sp + 10000;
	if (spNew < 0)
		spNew = 0;
	int32_t spFinal = 0;
	while ((spNew - 111) > 0) {
		spFinal += 1;
		spNew -= 111;
	}
	int32_t error = ((int32_t) state.controlPitch - spFinal;
	state.controlPitch = (100 * eps)/100 + 90;

}
*/

void controlComponentLoop() {
	// TODO: implement
}

void update_motors(void)
{
	for (int i = 0; i<4 ; i++){
		if (ae[i] > 550){
			ae[i] = 550;
		}
	}
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	//controlLift: 0-1000
	//controlPitch, controlRoll, controlYaw: 0-180
	uint32_t controlPitch = state.controlPitch;
	uint32_t controlRoll = state.controlRoll;
	uint32_t controlYaw = state.controlYaw;
	uint32_t controlLift = state.controlLift;
	ae[0] = controlLift;
	ae[1] = controlLift;
	ae[2] = controlLift;
	ae[3] = controlLift;
	if (controlLift > 180) {
		if (controlPitch > 90) {
			ae[0] += controlPitch - 90;
			ae[2] -= controlPitch - 90;
		}else {
			ae[0] -= 90 - controlPitch;
			ae[2] += 90 - controlPitch;
		}
		if (controlRoll > 90) {
			ae[1] += controlRoll - 90;
			ae[3] -= controlRoll - 90;
		}else{
			ae[1] -= 90 - controlRoll;
			ae[3] += 90 - controlRoll;
		}
		if (controlYaw > 90){
			ae[0] -= controlYaw - 90;
			ae[1] += controlYaw - 90;
			ae[2] -= controlYaw - 90;
			ae[3] += controlYaw - 90;
		} else{
			ae[0] += 90 - controlYaw;
			ae[1] -= 90 - controlYaw;
			ae[2] += 90 - controlYaw;
			ae[3] -= 90 - controlYaw;
		}
	}

	update_motors();
}

void manualControlBackup()
{
	uint8_t setMotors = 0;
	int32_t adjust1 = 640, adjust2 = 1700, adjust3 = 3500, b = 1, d = 1;
	int32_t aeSQ[4];
	int32_t Z = state.controlLift;
	int32_t L = state.controlPitch - 90;
	int32_t M = state.controlRoll - 90;
	int32_t N = state.controlYaw - 90;

	Z *= adjust1; L *= adjust2; M *= adjust2; N *= adjust3;
	aeSQ[0] = Z/(4*b) + L/(2*b) - N/(4*d);
	aeSQ[1] = Z/(4*b) - M/(2*b) + N/(4*d);
	aeSQ[2] = Z/(4*b) - L/(2*b) - N/(4*d);
	aeSQ[3] = Z/(4*b) + M/(2*b) + N/(4*d);
	for (int i = 0; i < 4; i++){
		if (aeSQ[i] < 0)
			setMotors = 1;
		ae[i] = root(aeSQ[i], 2) + 120;
	}
	if (setMotors == 0)
		update_motors();
}
