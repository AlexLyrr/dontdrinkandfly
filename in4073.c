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
	ble_init();

	state.currentMode = 0;
	state.nextMode = 0;

	state.controlChanged = false;

	state.hasPacket = false;
	state.sendStatus = false;
	state.sendAck = false;
	state.sendMotorStatus = false;
	state.sendTimings = false;
	state.packetError = 0;

	state.pChanged = false;
	state.pRoll = 1;
	state.pPitch = 1;
	state.pYaw = 1;

	state.calibrated = false;
	state.calibratePhiOffset = 0;
	state.calibrateThetaOffset = 0;
	state.calibratePsiOffset = 0;

	int32_t panicStep = 0;
	systemDone = false;
	appClock = 0;
	uint32_t start;

	while (!systemDone) {
		start = get_time_us();

		communicationComponentLoop();
		packetComponentLoop();
		
		state.timeLoopPacket = start - get_time_us();
		if (state.timeLoopPacket > state.timeLoopPacketMax) {
			state.timeLoopPacketMax = state.timeLoopPacket;
		}

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
				if (state.calibrated) {
					state.nextMode = 0;
				}
				break;
			case 4: // Manual Yaw
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					yawControl();
					run_filters_and_control();
					writeMotorStatus();
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
					rollControl();
					run_filters_and_control();
					writeMotorStatus();
				}
				if (state.controlChanged) { // We don't need to do anything extra yet when this happens
					state.controlChanged = false;
				}
				if (state.pChanged) { // We don't need to do anything extra yet when this happens
					state.pChanged = false;
				}
				break;	
			case 6:
				if (state.controlChanged || state.pChanged || check_sensor_int_flag()) {
					pitchControl();
					run_filters_and_control();
					writeMotorStatus();
				}
				if (state.controlChanged) { // We don't need to do anything extra yet when this happens
					state.controlChanged = false;
				}
				if (state.pChanged) { // We don't need to do anything extra yet when this happens
					state.pChanged = false;
				}
				break;
		}

		state.timeLoopControl = start - get_time_us() - state.timeLoopPacket;
		if (state.timeLoopControl > state.timeLoopControlMax) {
			state.timeLoopControlMax = state.timeLoopControl;
		}

		if (check_timer_flag()) {
			if (appClock%2 == 0) {
				nrf_gpio_pin_toggle(YELLOW);
			}
			if (appClock%100 == 0) { // Every second
				nrf_gpio_pin_toggle(BLUE);
			}
			if (appClock%1000 == 0) {
				state.sendStatus = true;
				state.sendTimings = true;
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

			
		}
		state.timeLoopApp = start - get_time_us() - state.timeLoopPacket - state.timeLoopControl;
		if (state.timeLoopApp > state.timeLoopAppMax) {
			state.timeLoopAppMax = state.timeLoopApp;
		}

		if (check_sensor_int_flag()) {
			
			get_dmp_data();
			if (state.currentMode == 3 && !state.calibrated) { // Calibrate mode
				state.calibratePhiOffset = phi;
				state.calibrateThetaOffset = theta;
				state.calibratePsiOffset = psi;
				state.calibrated = true;
			}
			controlComponentLoop();
		}

		state.timeLoopSensor = start - get_time_us() - state.timeLoopPacket - state.timeLoopControl - state.timeLoopApp;
		if (state.timeLoopSensor > state.timeLoopSensorMax) {
			state.timeLoopSensorMax = state.timeLoopSensor;
		}

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
			writeTimings();
		}
		state.timeLoop = start - get_time_us();
		if (state.timeLoop > state.timeLoopMax) {
			state.timeLoopMax = state.timeLoop;
		}	
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
