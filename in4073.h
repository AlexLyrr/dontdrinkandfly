/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

#include "app_util_platform.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RED 22
#define YELLOW 24
#define GREEN 28
#define BLUE 30
#define INT_PIN 5
#define PREAMPLE_B1 0x13
#define PREAMPLE_B2 0x37
#define DEBUGGING

// Control
#define P2PHI 10
#define C1 6
#define C2 18
#define PRECISION 14

int16_t motor[4], ae[4];
void manual_control_motor();
void yawControl();
void pitchControl();
void rollControl();
void yawControlRaw();
void pitchControlRaw();
void rollControlRaw();

void kalmanRoll();
void kalmanPitch();
void full_control_motor();

// Filters
int32_t lowpassFilterYaw(int32_t x0);
int32_t lowpassFilterRoll(int32_t x0);
int32_t lowpassFilterPitch(int32_t x0);

int32_t butterWorth2nd(int32_t x0);
int32_t fixedPoint(int32_t x0);
int32_t notFixedPoint(int32_t x0);

void yawFilter();
void rollFilter();
void pitchFilter();
void kalmanRoll();
void kalmanPitch();

int32_t maPressureFilter();

int32_t phiFiltered, thetaFiltered, psiFiltered;
int32_t spFiltered, sqFiltered, srFiltered;
int32_t saxFiltered, sayFiltered, sazFiltered;
int32_t pAfterKalman, phiAfterKalman, qAfterKalman, thetaAfterKalman;

// Timers
#define TIMER_PERIOD 10 //50ms=20Hz (MAX 23bit, 4.6h)
void timers_init(void);
uint32_t get_time_us(void);
bool check_timer_flag(void);
void clear_timer_flag(void);

// GPIO
void gpio_init(void);

// Queue
#define QUEUE_SIZE 256
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint16_t first, last;
	uint16_t count;
} queue;
void init_queue(queue* q);
void enqueue(queue* q, uint8_t x);
uint8_t dequeue(queue* q);
uint8_t queuePeek(queue* q, uint16_t offset);

// UART
#define RX_PIN_NUMBER 16
#define TX_PIN_NUMBER 14
queue rx_queue;
queue tx_queue;
uint32_t last_correct_checksum_time;
void uart_init(void);
void uart_put(uint8_t);

// TWI
#define TWI_SCL 4
#define TWI_SDA 2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const* data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t* data);

// MPU wrapper
int16_t phi, theta, psi;
int16_t sp, sq, sr;
int16_t sax, say, saz;

int32_t phi_fp, theta_fp;
int32_t sp_fp, sq_fp;

uint8_t sensor_fifo_count;
void imu_init(bool dmp, uint16_t interrupt_frequency); // if dmp is true, the interrupt frequency is 100Hz - otherwise 32Hz-8kHz
void get_dmp_data(void);
void get_raw_sensor_data(void);
bool check_sensor_int_flag(void);
void clear_sensor_int_flag(void);

// Barometer
int32_t pressure;
int32_t temperature;
void read_baro(void);
void baro_init(void);

// ADC
uint16_t bat_volt;
void adc_init(void);
void adc_request_sample(void);

// Flash
bool spi_flash_init(void);
bool flash_chip_erase(void);
bool flash_write_byte(uint32_t address, uint8_t data);
bool flash_write_bytes(uint32_t address, uint8_t* data, uint32_t count);
bool flash_read_byte(uint32_t address, uint8_t* buffer);
bool flash_read_bytes(uint32_t address, uint8_t* buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
volatile bool radio_active;
void ble_init(void);
void ble_send(void);

/*******
	State
	@author Joseph Verburg
*******/
#define PANIC_STEPS (5000 / TIMER_PERIOD) // 5 seconds
#define CALIBRATION_STEPS (20000 / TIMER_PERIOD) // 20 seconds

#define PACKET_BODY_LENGTH 10
#define PACKET_LENGTH (PACKET_BODY_LENGTH + 5)
#define APPLICATION_TIMINGS
#ifdef APPLICATION_TIMINGS
// #define APPLICATION_TIMINGS_EXTENDED
#endif
#define BLE_ENABLED

typedef struct {
	uint8_t nextMode;
	uint8_t currentMode;

	// Packet component
	bool hasPacket;
	bool sendStatus;
	bool sendMotorStatus;
	bool sendTimings;
	uint8_t packetError;
	uint16_t packetAck;
	bool sendAck;
	bool sendPing;
	uint32_t lastPacketReceived;
	uint16_t packetNumber;
	uint8_t currentPacket[PACKET_LENGTH];

	// Panic mode
	uint16_t panicFinished; // In appClock;
	uint32_t panicMotor[4];

	uint8_t motor1Offset;
	uint8_t motor2Offset;
	uint8_t motor3Offset;
	uint8_t motor4Offset;

	// Control params
	bool dmpEnabled;
	bool pChanged;
	uint16_t p1;
	uint16_t p2;
	uint16_t pYaw;
	uint16_t pLift;
	uint8_t psaz;
	bool controlChanged;
	uint16_t controlRoll;
	uint16_t controlPitch;
	uint16_t controlYaw;
	uint16_t controlLift;
	uint8_t controlYawUser;
	uint8_t controlPitchUser;
	uint8_t controlRollUser;
	uint16_t controlLiftUser;

	// Calibration
	bool calibrated;
	uint32_t calibrationFinished; // In appClock
	int32_t calibratePhiOffset;
	int32_t calibrateThetaOffset;
	int32_t calibratePsiOffset;
	int32_t calibrateSpOffset;
	int32_t calibrateSqOffset;
	int32_t calibrateSrOffset;
	int32_t calibrateSaxOffset;
	int32_t calibrateSayOffset;
	int32_t calibrateSazOffset;
	int32_t calibratePressureOffset;

	bool heightSet;
	int32_t initPressure;

	uint32_t timeLoopTotal;
	uint32_t timeLoopCount;
	uint32_t timeLoopMax;
#ifdef APPLICATION_TIMINGS_EXTENDED
	uint32_t timeLoopControlTotal;
	uint32_t timeLoopControlCount;
	uint32_t timeLoopControlMax;

#endif
} State;
State state;
bool systemDone;
uint16_t appClock;

void onAbort();

/*******
    Application & Components
	@author Joseph Verburg
*******/
void communicationComponentLoop();
void packetComponentLoop();
void applicationComponentLoop();
bool hasNonZeroControl();

/*******
	height control
	@author Roy Blokker
*******/

void init_height();
void heightControl();

/*******
	Protocol
	@author Joseph Verburg
*******/
void writeByte(uint8_t b);
void writePacket(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8, uint8_t b9);
void parsePacketInit();
void parsePacketSetControl();
void parsePacketSetMode();
void parsePacketSetP();
void parsePacketPing();

void writeDroneStatus();
void writeError(uint8_t errorCode);
void writeMotorStatus();
void writeAck(uint16_t packetNumber);
void writeTimings();
void writePing();
void writeSensorValues();
void writeOffsetValues();
void writeRawValues(uint16_t sp, uint16_t sq, uint16_t sr, uint16_t sax, uint16_t say, uint16_t saz);

int root(int a, int n);
int iPow(int a, int e);
void manualControlBackup();
#endif // IN4073_H__
