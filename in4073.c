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

/**
 *	@author Roy Blokker
 */
#define MIN_PACKET_INTERVAL_US (2 * 1000 * 1000)
void checkSafety() {
	uint32_t currentTime = get_time_us();
	if (state.currentMode != 0 && (currentTime - state.lastPacketReceived) > MIN_PACKET_INTERVAL_US) {
	// 	nrf_gpio_pin_toggle(GREEN);
	// 	timeLastPacket = get_time_us();
	// } else {
		state.nextMode = 1;
	}
	if (bat_volt < 155 && bat_volt > 128){ // with drone
		state.nextMode = 1;
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
	imu_init(true, 100);
	baro_init();
	spi_flash_init();
	// ble_init();

	state.currentMode = 0;
	state.nextMode = 0;

	state.controlChanged = false;

	state.hasPacket = false;
	state.sendStatus = false;
	state.sendAck = false;
	state.sendMotorStatus = false;
	state.sendTimings = false;
	state.sendPing = false;

	state.packetError = 0;

	state.pChanged = false;
	state.p1 = 6;
	state.p2 = 5;
	state.pYaw = 50;

	state.calibrated = false;
	state.calibratePhiOffset = 0;
	state.calibrateThetaOffset = 0;
	state.calibratePsiOffset = 0;

	int32_t panicStep = 0;
	systemDone = false;
	appClock = 0;

	while (!systemDone) {
		#ifdef APPLICATION_TIMINGS
		start = get_time_us();
		#endif

		communicationComponentLoop();
		packetComponentLoop();
		
		#ifdef APPLICATION_TIMINGS
		state.timeLoopPacket = get_time_us() - start;
		if (state.timeLoopPacket > state.timeLoopPacketMax) {
			state.timeLoopPacketMax = state.timeLoopPacket;
		}
		#endif

		switch(state.currentMode) {
			case 0:
				motor[0] = 0;
				motor[1] = 0;
				motor[2] = 0;
				motor[3] = 0;
				break;
			case 1: // Panic!
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
					run_filters_and_control(); // TODO: rename function
					state.controlChanged = false;
					state.sendMotorStatus = true;
					// writeMotorStatus(); // TODO: move to the end of the control loop
				}
				break;
			case 3: // Calibration
				if (check_sensor_int_flag()) {
					get_dmp_data();
					state.calibratePhiOffset = phi;
					state.calibrateThetaOffset = theta;
					state.calibratePsiOffset = psi;
					state.calibrateSpOffset = sp;
					state.calibrateSqOffset = sq;
					state.calibrateSrOffset = sr;
					state.calibrated = true;
					state.nextMode = 0;
				}
				break;
			case 4: // Manual Yaw
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					yawControl();
					run_filters_and_control();
					#ifdef DEBUGGING
					state.sendMotorStatus = true;
					#endif
				}
				if (state.controlChanged) { // We don't need to do anything extra yet when this happens
					state.controlChanged = false;
				}
				if (state.pChanged) { // We don't need to do anything extra yet when this happens
					state.pChanged = false;
				}
				break;
			case 5:
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					yawControl();
					rollControl();
					pitchControl();
					run_filters_and_control();
					#ifdef DEBUGGING
					state.sendMotorStatus = true;
					#endif
				}
				if (state.controlChanged) { // We don't need to do anything extra yet when this happens
					state.controlChanged = false;
				}
				if (state.pChanged) { // We don't need to do anything extra yet when this happens
					state.pChanged = false;
				}
				break;	
			case 9:
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					if (check_sensor_int_flag()) {
						get_dmp_data();
					}
					//rollControl();
					pitchControl();
					run_filters_and_control();
					#ifdef DEBUGGING
					state.sendMotorStatus = true;
					#endif
				}
				if (state.controlChanged) { // We don't need to do anything extra yet when this happens
					state.controlChanged = false;
				}
				if (state.pChanged) { // We don't need to do anything extra yet when this happens
					state.pChanged = false;
				}
				break;
		}
		#ifdef APPLICATION_TIMINGS
		state.timeLoopControl = get_time_us() - start - state.timeLoopPacket;
		if (state.timeLoopControl > state.timeLoopControlMax) {
			state.timeLoopControlMax = state.timeLoopControl;
		}
		#endif

		if (check_timer_flag()) {
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
				state.sendPing = true;
				#endif
			}
			if (appClock%5 == 0) {
				adc_request_sample();
				read_baro();
			}
			if (state.currentMode == 0 && appClock%20 == 0) {
				if (check_sensor_int_flag()) {
					get_dmp_data();
				}
				writeRawValues();
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
						switch(state.nextMode) {
							case 1:
								state.panicFinished = appClock + PANIC_STEPS;
								for(uint8_t i = 0; i < 4; i += 1) {
										state.panicMotor[i] = motor[i];
								}
								break;
						}
						state.currentMode = state.nextMode;
						break;
				}
				state.sendStatus = true;
			}
			if (state.currentMode == 1) {
				if (appClock%5 == 0) {
					state.sendStatus = true;
					state.sendMotorStatus = true;
				}
				if (state.panicFinished == appClock) {
					state.currentMode = 0;
					systemDone = true;
					state.sendStatus = true;
				}
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

			checkSafety();
		}

		#ifdef APPLICATION_TIMINGS
		state.timeLoopApp = get_time_us() - start - state.timeLoopPacket - state.timeLoopControl;
		if (state.timeLoopApp > state.timeLoopAppMax) {
			state.timeLoopAppMax = state.timeLoopApp;
		}
		#endif

		#ifdef APPLICATION_TIMINGS
		state.timeLoopSensor = get_time_us() - start - state.timeLoopPacket - state.timeLoopControl - state.timeLoopApp;
		if (state.timeLoopSensor > state.timeLoopSensorMax) {
			state.timeLoopSensorMax = state.timeLoopSensor;
		}
		#endif
		
		#ifdef APPLICATION_TIMINGS
		state.timeLoop = get_time_us() - start;
		if (state.timeLoop > state.timeLoopMax) {
			state.timeLoopMax = state.timeLoop;
		}	
		#endif
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
