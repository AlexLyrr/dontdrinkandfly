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


//@Author Alex Lyrakis
void yawControl() {
	int32_t eps = (((int32_t) state.controlYawUser - 90) << 2) + (((int32_t) sr - state.calibrateSrOffset) >> 2);
	int32_t yawValue = ((state.pYaw * eps) >> 8) + 180;
	if (yawValue > 360)
		yawValue = 360;
	if (yawValue < 0)
		yawValue = 0;
	state.controlYaw = (uint16_t) yawValue;
}

//@Author Alex Lyrakis
void pitchControl(){
	int32_t eps = (((int32_t) state.controlPitchUser - 90) << 2) - ((theta - state.calibrateThetaOffset) >> 2); 
	int32_t pitchValue = (state.p1 * eps);
	int32_t eps2 = (state.p2 *((sq - state.calibrateSqOffset) >> 2)) + pitchValue;
	pitchValue = (eps2 >> 8) + 180;
	if (pitchValue > 360)
		pitchValue = 360;
	if (pitchValue < 0)
		pitchValue = 0;
	state.controlPitch = (uint16_t) pitchValue;
}

//@Author Alex Lyrakis
void rollControl(){
	int32_t eps = (((int32_t) state.controlRollUser - 90) << 2) + ((phi - state.calibratePhiOffset) >> 2);
	int32_t rollValue = (state.p1 * eps);
	int32_t eps2 = (state.p2 * ((sp - state.calibrateSpOffset) >> 2)) + rollValue;
	rollValue = (eps2 >> 8) + 180;
	if (rollValue > 360)
		rollValue = 360;
	if (rollValue < 0)
		rollValue = 0;
	state.controlRoll = (uint16_t) rollValue;
}


//@Author Alex Lyrakis
void yawControlRaw() {
	int32_t eps = (((int32_t) state.controlYawUser - 90) << 2) + (srFiltered);
	int32_t yawValue = ((state.pYaw * eps) >> 8) + 180;
	if (yawValue > 360)
		yawValue = 360;
	if (yawValue < 0)
		yawValue = 0;
	state.controlYaw = (uint16_t) yawValue;
}

//@Author Alex Lyrakis
void pitchControlRaw(){
	int32_t eps = (((int32_t) state.controlPitchUser - 90) << 2) - (thetaFiltered >> 2); 
	int32_t pitchValue = (state.p1 * eps);
	int32_t eps2 = (state.p2 *(sqFiltered >> 2)) + pitchValue;
	pitchValue = (eps2 >> 8) + 180;
	if (pitchValue > 360)
		pitchValue = 360;
	if (pitchValue < 0)
		pitchValue = 0;
	state.controlPitch = (uint16_t) pitchValue;
}

//@Author Alex Lyrakis
void rollControlRaw(){
	int32_t eps = (((int32_t) state.controlRollUser - 90) << 2) + (phiFiltered >> 2);
	int32_t rollValue = (state.p1 * eps);
	int32_t eps2 = (state.p2 * (spFiltered >> 2)) + rollValue;
	rollValue = (eps2 >> 8) + 180;
	if (rollValue > 360)
		rollValue = 360;
	if (rollValue < 0)
		rollValue = 0;
	state.controlRoll = (uint16_t) rollValue;
}


//@Author Alex Lyrakis
void kalmanRoll(){

	static int32_t pKalman = 0, spPrev = 0, pBias = 0, pBiasPrev = 0, phiKalman = 0, phiKalmanPrev = 0, phiError = 0;

	pKalman = spPrev - pBiasPrev;
	phiKalman = phiKalmanPrev + (pKalman >> P2PHI); 
	phiError = phiKalman - phi;
	phiKalman = phiKalman - (phiError >> C1);
	pBias = pBiasPrev + ((phiError >> P2PHI) >> C2);  
	
	phiKalmanPrev = phiKalman;
	pBiasPrev = pBias;
	spPrev = (int32_t) sp; // get the value from sensor
}

void kalmanPitch(){

}

void controlComponentLoop() {
	// TODO: implement
}

//@Author Georgios Giannakaras
//fc = 1HZ fs = 1000
int32_t lowpassFilter(int32_t x0){
    static int32_t x1 = 0, x2 = 0, y0, y1 = 0, y2 = 0;
    
    y0 = (x0 >> 17) + (x0 >> 19) + (x1 >> 16) + (x1 >> 18) + (x1 >> 20)
     + (x2 >> 17) + (x2 >> 19)
      + ((y1) + (y1 >> 1) + (y1 >> 2) + (y1 >> 3) + (y1 >> 4) + (y1 >> 5) + (y1 >> 6) + (y1 >> 8) + (y1 >> 9) 
        + (y1 >> 11) + (y1 >> 12) + (y1 >> 13) + (y1 >> 16) + (y1 >> 17) + (y1 >> 19) + (y1 >> 20)) 
      - ((y2 >> 1) + (y2 >> 2) + (y2 >> 3) + (y2 >> 4) + (y2 >> 5) + (y2 >> 6) + (y2 >> 8) + (y2 >> 9)
       + (y2 >> 11) + (y2 >> 12) + (y2 >> 13) + (y2 >> 14) + (y2 >> 18));
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0;
}


//@Author Georgios Giannakaras
//fc = 20HZ fs = 1000
int32_t butterWorth2nd(int32_t x0){
    static int32_t x1 = 0, x2 = 0, y0, y1 = 0, y2 = 0;

    y0 = (x0 >> 9) + (x0 >> 10) + (x0 >> 11) + (x0 >> 13) + (x0 >> 14) + (x0 >> 16) + (x0 >> 18) + (x0 >> 19) + (x1 >> 8) + (x1 >> 9) +
     (x1 >> 10) + (x1 >> 12) + (x1 >> 13) + (x1 >> 15) + (x1 >> 17) + (x1 >> 19) + (x1 >> 20) + (x2 >> 9) + (x2 >> 10) + (x2 >> 11) + 
     (x2 >> 13) + (x2 >> 14) + (x2 >> 16) + (x2 >> 18) + (x2 >> 19) + ((y1) + (y1 >> 1) + (y1 >> 2) + (y1 >> 4) + (y1 >> 7) + (y1 >> 9) + 
      (y1 >> 12) + (y1 >> 13) + (y1 >> 14) + (y1 >> 19)) - ((y2 >> 1) + (y2 >> 2) + (y2 >> 4) + (y2 >> 6) + (y2 >> 7) + (y2 >> 10) + (y2 >> 12) + 
      (y2 >> 16) + (y2 >> 17) + (y2 >> 20));

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0;
}

int32_t fixedPoint(int32_t x0){
  return (x0 << PRECISION);
}

void yawFilter(){
	int32_t x0, y0calibrated;
	x0 = fixedPoint(sr);
    y0calibrated = x0 - lowpassFilter(x0);
    y0Butter = butterWorth2nd(y0calibrated);  
    srFiltered = (int16_t) (y0Butter >> PRECISION);
}

void update_motors(void)
{
	for (int i = 0; i<4 ; i++){
		if (ae[i] > 700){
			ae[i] = 700;
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
		} else {
			ae[0] -= 90 - controlPitch;
			ae[2] += 90 - controlPitch;
		}
		if (controlRoll > 90) {
			ae[1] += controlRoll - 90;
			ae[3] -= controlRoll - 90;
		} else{
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

void full_control_motor()
{
	uint32_t controlPitch = state.controlPitch;
	uint32_t controlRoll = state.controlRoll;
	uint32_t controlYaw = state.controlYaw;
	uint32_t controlLift = state.controlLift;
	ae[0] = controlLift;
	ae[1] = controlLift;
	ae[2] = controlLift;
	ae[3] = controlLift;
	if (controlLift > 180) {
		if (controlPitch > 180) {
			ae[0] += (controlPitch - 180);
			ae[2] -= (controlPitch - 180);
		} else {
			ae[0] -= (180 - controlPitch);
			ae[2] += (180 - controlPitch);
		}
		if (controlRoll > 180) {
			ae[1] += (controlRoll - 180);
			ae[3] -= (controlRoll - 180);
		} else{
			ae[1] -= (180 - controlRoll);
			ae[3] += (180 - controlRoll);
		}
		if (controlYaw > 180){
			ae[0] -= (controlYaw - 180);
			ae[1] += (controlYaw - 180);
			ae[2] -= (controlYaw - 180);
			ae[3] += (controlYaw - 180);
		} else{
			ae[0] += (180 - controlYaw);
			ae[1] -= (180 - controlYaw);
			ae[2] += (180 - controlYaw);
			ae[3] -= (180 - controlYaw);
		}
	}

	update_motors();
}
