/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"

/*
 *
 * @author Joseph Verburg
 */
void onAbort() {
	switch(state.currentMode) {
		case 0:
			systemDone = true;
			break;
		case 1:
			break;
		default:
			state.nextMode = 1;
			// state.panicFinished = appClock + (5000 / TIMER_PERIOD);
			break;
	}
	state.sendStatus = true;
}

bool hasNonZeroControl() {
	return state.controlLiftUser != 0;
}

/**
 *	@author Roy Blokker
 */
#define MIN_PACKET_INTERVAL_US (2 * 1000 * 1000)
#define MIN_PACKET_INTERVAL_US_BLE (10 * 1000 * 1000)
void checkSafety() {
	if (state.currentMode != state.nextMode) {
		if (state.currentMode == 0 && state.nextMode != 1 && hasNonZeroControl()) {
			state.nextMode = state.currentMode;
		}
	}

	uint32_t currentTime = get_time_us();
	if (state.currentMode == 8) {
		if ((currentTime - state.lastPacketReceived) > MIN_PACKET_INTERVAL_US_BLE) {
			state.nextMode = 1;
		}
	} else {
		if (state.currentMode != 0 && (currentTime - state.lastPacketReceived) > MIN_PACKET_INTERVAL_US) {
			state.nextMode = 1;
		}
	}

	/*
		This is raw battery value.
		real value = (bat_volt * 7) / 100;
		143 = 10.01 V
		This is the value where it goes into panic mode

		128 = 8.96 V 
		This is to check we have a connected battery
	*/
	if (bat_volt < 143 && bat_volt > 128){
		state.nextMode = 1;
	}
}

/**
 *	@author Joseph Verburg 
 */
void applicationComponentLoop() {
	checkSafety();

	if (appClock%2 == 0) {
		nrf_gpio_pin_toggle(YELLOW);
	}
	if (appClock%100 == 0) { // Every second
		nrf_gpio_pin_toggle(BLUE);
	}
	if (appClock%200 == 0) {
		state.sendStatus = true;
		#ifndef DEBUGGING
		state.sendMotorStatus = true;
		#endif
		#ifdef APPLICATION_TIMINGS
		state.sendTimings = true;
		#endif
	}
	if (appClock%5 == 0) {
		adc_request_sample();
		read_baro();
	}

	if (state.nextMode != state.currentMode) {
		switch(state.currentMode) {
			case 1:
				state.nextMode = 1;
				break;
			case 3:
				state.calibrated = false; // reset flag
				state.currentMode = state.nextMode;
				break;
			case 0:
				if (state.nextMode == 1) {
					systemDone = true;
					break;
				}
			default:
				if (state.currentMode == 7) {
					state.heightSet = false;
				}
				if (state.nextMode == 7 && state.currentMode != 5) {
					state.nextMode = state.currentMode;
					break;
				}
				switch(state.nextMode) {
					case 1:
						state.panicFinished = appClock + PANIC_STEPS;
						for(uint8_t i = 0; i < 4; i += 1) {
								state.panicMotor[i] = motor[i];
						}
						break;
					case 3:
						state.calibrationFinished = appClock + CALIBRATION_STEPS;
						if (!state.dmpEnabled) {
							imu_init(true, 100);
							state.dmpEnabled = true;
						}
						state.calibrated = false;
						state.calibratePhiOffset = 0;
						state.calibrateThetaOffset = 0;
						state.calibratePsiOffset = 0;
						state.calibrateSpOffset = 0;
						state.calibrateSqOffset = 0;
						state.calibrateSrOffset = 0;
						state.calibrateSaxOffset = 0;
						state.calibrateSayOffset = 0;
						state.calibrateSazOffset = 0;
						state.calibratePressureOffset = 0;
						
						// dmp_enable_gyro_cal(1);
						break;
					case 2:
					case 4:
					case 5:
					case 7:
					case 8:
						if (!state.dmpEnabled) {
							imu_init(true, 100);
							state.dmpEnabled = true;
						}
						break;
					case 6:
						if (!state.dmpEnabled) {
							imu_init(false, 500);
							state.dmpEnabled = true;
						}
						break;
				}
				state.currentMode = state.nextMode;
				break;
		}
		state.sendStatus = true;
	}

	switch(state.currentMode) {
		case 0:
			if (appClock%20 == 0) {
				if (state.dmpEnabled && check_sensor_int_flag()) {
					get_dmp_data();
					writeSensorValues();
				}
			}
			break;
		case 1:
			if (appClock%5 == 0) {
				state.sendStatus = true;
				state.sendMotorStatus = true;
			}
			if (state.panicFinished == appClock) {
				state.currentMode = 0;
				systemDone = true;
				state.sendStatus = true;
			}
			break;
		case 3:
			// if (appClock%20 == 0) {
				writeSensorValues();
			// }
			if (state.calibrationFinished == appClock) {
				// dmp_enable_gyro_cal(0);
				writeOffsetValues();
				writeMotorStatus();
				//init_height();
				//state.heightSet = true;
				state.calibrated = true;
				state.nextMode = 0;
				state.currentMode = 0;
				state.sendStatus = true;
			}
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			#ifdef DEBUGGING
			if (appClock % 4 == 0) { // 25 FPS
				state.sendMotorStatus = true;
			}
			#endif
			break;
	}

	clear_timer_flag();
	appClock++;

	if (state.packetError != 0) {
		writeError(state.packetError);
		state.packetError = 0;
	} else if (state.sendAck) {
		state.sendAck = false;
		writeAck(state.packetAck);
		state.packetAck = 0;
	} else if (state.sendStatus) {
		state.sendStatus = false;
		writeDroneStatus();
	} else if (state.sendMotorStatus) {
		state.sendMotorStatus = false;
		writeMotorStatus();
	} else if (state.sendTimings) {
		state.sendTimings = false;
		#ifdef APPLICATION_TIMINGS
		writeTimings();
		#endif
	} else if (state.sendPing) {
		state.sendPing = false;
		writePing(get_time_us());
	}
}


/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	baro_init();
	spi_flash_init();
	
	#ifdef BLE_ENABLED
	ble_init();
	#endif

	state.currentMode = 0;
	state.nextMode = 0;

	state.dmpEnabled = false;
	state.controlChanged = false;

	state.hasPacket = false;
	state.sendStatus = false;
	state.sendAck = false;
	state.sendMotorStatus = false;
	state.sendTimings = false;
	state.sendPing = false;

	state.packetError = 0;

	state.pChanged = false;
	state.p1 = 25;
	state.p2 = 25;
	state.pYaw = 80;
	state.pLift = 0;
	state.psaz = 0;

	state.calibrated = false;
	state.heightSet = false;

	state.controlLiftUser = 0;
	state.controlPitchUser = 90;
	state.controlRollUser = 90;
	state.controlYawUser = 90;

	int32_t panicStep = 0;
	systemDone = false;
	appClock = 0;
	#ifdef APPLICATION_TIMINGS
	int32_t loopStart = 0, loopLength = 0;
	bool workDone = false;
	#endif

	while (!systemDone) {
		#ifdef APPLICATION_TIMINGS
		loopStart = get_time_us();
		workDone = false;
		#endif

		communicationComponentLoop();
		packetComponentLoop();

		switch(state.currentMode) {
			case 0:
				motor[0] = 0;
				motor[1] = 0;
				motor[2] = 0;
				motor[3] = 0;
				break;
			case 1: // Panic!
				#ifdef APPLICATION_TIMINGS
				workDone = true;
				#endif
				panicStep = state.panicFinished - appClock;
				if (panicStep < 0) {
						panicStep = 0;
				}
				motor[0] = (uint16_t)(state.panicMotor[0] * panicStep / PANIC_STEPS);
				motor[1] = (uint16_t)(state.panicMotor[1] * panicStep / PANIC_STEPS);
				motor[2] = (uint16_t)(state.panicMotor[2] * panicStep / PANIC_STEPS);
				motor[3] = (uint16_t)(state.panicMotor[3] * panicStep / PANIC_STEPS);
				break;
			case 2: // Manual
				if (state.controlChanged) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					run_filters_and_control(); // TODO: rename function
					state.controlChanged = false;
					state.sendMotorStatus = true;
					// writeMotorStatus(); // TODO: move to the end of the control loop
				}
				break;
			case 3: // Calibration
				if (check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					get_dmp_data();
					// Low pass filtering
					state.calibratePhiOffset = (state.calibratePhiOffset * 7 + phi) >> 3;
					state.calibrateThetaOffset = (state.calibrateThetaOffset * 7 + theta) >> 3;
					state.calibratePsiOffset = (state.calibratePsiOffset * 7 + psi) >> 3;
					state.calibrateSpOffset = (state.calibrateSpOffset * 7  + sp) >> 3;
					state.calibrateSqOffset = (state.calibrateSqOffset * 7 + sq) >> 3;
					state.calibrateSrOffset = (state.calibrateSrOffset * 7 + sr) >> 3;
					state.calibrateSaxOffset = (state.calibrateSaxOffset * 7 + sax) >> 3;
					state.calibrateSayOffset = (state.calibrateSayOffset * 7 + say) >> 3;
					state.calibrateSazOffset = (state.calibrateSazOffset * 7 + saz) >> 3;
					state.calibratePressureOffset = (state.calibratePressureOffset * 7 + pressure) >> 3;
				}
				break;
			case 4: // Manual Yaw
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					yawControl();
					run_filters_and_control();
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
			case 5: // Full-Control
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					yawControl();
					rollControl();
					pitchControl();
					full_control_motor();
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
			case 6: // Raw-control
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					get_raw_sensor_data();
					// if ((get_time_us() - recording_last) > MIN_RECORD_TIME){
					yawFilter();
					rollFilter();
					pitchFilter();
					yawControlRaw();
					rollControlRaw();
					pitchControlRaw();
					full_control_motor();
						// recording_last = get_time_us();
					// }
					
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
			case 7: //Height control
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					if (state.heightSet == false){
						for (int i = 0; i < 4; i++){
							init_height();
							state.heightSet = true;
							if (check_sensor_int_flag()) {
								get_dmp_data();
							}
						}
					}
					heightControl2();
					yawControl();
					rollControl();
					pitchControl();
					full_control_motor();
					#ifdef DEBUGGING
					state.sendMotorStatus = true;
					#endif
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
			case 8: // Wireless
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					yawControl();
					rollControl();
					pitchControl();
					full_control_motor();
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
			case 9:
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					#ifdef APPLICATION_TIMINGS
					workDone = true;
					#endif
					//rollControl();
					pitchControl();
					run_filters_and_control();
					#ifdef DEBUGGING
					state.sendMotorStatus = true;
					#endif
					state.controlChanged = false;
					state.pChanged = false;
				}
				break;
		}

		if (check_timer_flag()) {
			#ifdef APPLICATION_TIMINGS
			workDone = true;
			#endif
			applicationComponentLoop();
		}
		#ifdef APPLICATION_TIMINGS
		loopLength = get_time_us() - loopStart;
		if (loopLength > 0 && workDone) {
			if (state.timeLoopMax < loopLength) {
				state.timeLoopMax = loopLength;
			}
			state.timeLoopTotal += loopLength;
			state.timeLoopCount++;
		}
		#endif
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
