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

void restoreDisp(){
	// TODO
}

void yawControl() {
		uint16_t displacement;
		if (prevTime == 0){
			displacement = 0;
		}
		else{
			if (sr > 10 || sr < -10){
				displacement += (get_time_us() - prevTime) * sr;
			}
		}
		restoreDisp();
		prevTime = get_time_us();
}

void controlComponentLoop() {
	// TODO: implement
}

void update_motors(void)
{
	for (int i = 0; i<4 ; i++){
		if (ae[i] > 400){
			ae[i] = 400;
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
	uint16_t adjust = 10000;
	uint32_t controlPitch = adjust * state.controlPitch;
	uint32_t controlRoll = adjust * state.controlRoll;
	uint32_t controlYaw = adjust * state.controlYaw;
	uint32_t controlLift = state.controlLift;
	ae[0] = (((controlPitch+controlYaw)/180) * controlLift)/adjust;
	ae[1] = (((controlRoll + (180*adjust-controlYaw))/180) * controlLift)/adjust;
	ae[2] = ((((180*adjust - controlPitch)+controlYaw)/180) * controlLift)/adjust;
	ae[3] = ((((180*adjust - controlRoll) + (180*adjust - controlYaw))/180) * controlLift)/adjust;
	for (int i = 0; i<4 ; i++){
		if (ae[i] > 1000){
			ae[i] = 1000;
		}
	}

	update_motors();
}

void manualControlBackup()
{
	int32_t adjust = 500, b = 1, d = 1;
	int32_t aeSQ[4];
	int32_t Z = state.controlLift*adjust;
	int32_t L = state.controlPitch - 90;
	int32_t M = state.controlRoll - 90;
	int32_t N = state.controlYaw - 90;

	Z *= adjust; L *= adjust/2; M *= adjust/2; N *= adjust/2;
	aeSQ[0] = Z/(4*b) + M/(2*b) - N/(4*d);
	aeSQ[1] = Z/(4*b) - L/(2*b) + N/(4*d);
	aeSQ[2] = Z/(4*b) - M/(2*b) - N/(4*d);
	aeSQ[3] = Z/(4*b) + L/(2*b) + N/(4*d);
	for (int i=0; i<4; i++)
		ae[i] = root(aeSQ[i], 2);

	update_motors();
}



