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

#include "../in4073.h"


/*
 * @author Joseph Verburg
 */
int main(void)
{
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(false, 1000);
	baro_init();
	spi_flash_init();

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
	state.p2 = 50;
	state.pYaw = 50;

	state.calibrated = false;
	state.calibratePhiOffset = 0;
	state.calibrateThetaOffset = 0;
	state.calibratePsiOffset = 0;

	systemDone = false;

	#define RECORD_DURATION (10 * 1000 * 1000)
	#define MIN_PACKET_DELAY (22 * 1000)
	bool recording = false;
	bool sending = false;
	uint32_t recording_start = 0;
	uint32_t last_packet_sent = get_time_us();

	uint32_t address = 0;
	uint8_t buffer[12]; 

	nrf_gpio_pin_set(YELLOW);
	nrf_gpio_pin_set(RED);
	nrf_gpio_pin_set(GREEN);
	nrf_gpio_pin_clear(BLUE);

	while (!systemDone) {
		if (!check_sensor_int_flag()) {
			continue;
		}
		communicationComponentLoop();
		get_raw_sensor_data();

		if (!recording) {
			if (state.hasPacket) {
				state.hasPacket = false;
				if (state.currentPacket[4] == 3) {
					nrf_gpio_pin_set(BLUE);
					recording = true;
					recording_start = get_time_us();
					nrf_gpio_pin_clear(GREEN);
				}
			}
		} else if (!sending && (get_time_us() - recording_start) < RECORD_DURATION) {
			if (address < 127000) { // Don't overflow the chip
				// do write
				buffer[0] = sp >> 8;
				buffer[1] = sp & 0xFF;
				buffer[2] = sq >> 8;
				buffer[3] = sq & 0xFF;
				buffer[4] = sr >> 8;
				buffer[5] = sr & 0xFF;

				buffer[6] = sax >> 8;
				buffer[7] = sax & 0xFF;
				buffer[8] = say >> 8;
				buffer[9] = say & 0xFF;
				buffer[10] = saz >> 8;
				buffer[11] = saz & 0xFF;
				
				flash_write_bytes(address, buffer, 12);

				address += 12;
			}
		} else {
			if (!sending) {
				nrf_gpio_pin_set(GREEN);
				nrf_gpio_pin_clear(YELLOW);
			}
			sending = true;
			if ((get_time_us() - last_packet_sent) > MIN_PACKET_DELAY) {
				address -= 12;

				flash_read_bytes(address, buffer, 12);
				
				writeRawValues(
					((uint16_t) buffer[0]) << 8 | ((uint16_t) buffer[1]),
					((uint16_t) buffer[2]) << 8 | ((uint16_t) buffer[3]),
					((uint16_t) buffer[4]) << 8 | ((uint16_t) buffer[5]),

					((uint16_t) buffer[6]) << 8 | ((uint16_t) buffer[7]),
					((uint16_t) buffer[8]) << 8 | ((uint16_t) buffer[9]),
					((uint16_t) buffer[10]) << 8 | ((uint16_t) buffer[11])
				);
				
				last_packet_sent = get_time_us();
				if (address <= 0) {
					nrf_gpio_pin_set(YELLOW);
					break;
				}
			}
		}


		// state.rawSp = toFixedPoint(sp);
		// state.rawSq = toFixedPoint(sq);
		// state.rawSr = toFixedPoint(sr);

		// state.rawSax = toFixedPoint(sax);
		// state.rawSay = toFixedPoint(say);
		// state.rawSaz = toFixedPoint(saz);
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(1000);

	NVIC_SystemReset();
}
