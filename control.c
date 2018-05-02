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



void controlComponentLoop() {
	// TODO: implement
}

void update_motors(void)
{
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
	ae[2] = ((((180*adjust - controlPitch)+controlYaw)/180) *controlLift)/adjust;
	ae[3] = ((((180*adjust - controlRoll) + (180*adjust - controlYaw))/180) * controlLift)/adjust;
	for (int i = 0; i<4 ; i++){
		if (ae[i] > 1000){
			ae[i] = 1000;
		}
	}

	update_motors();
}
