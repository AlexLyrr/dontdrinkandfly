#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef PC_TERMINAL_H_
#define PC_TERMINAL_H_

#define PREAMPLE_B1 0x13
#define PREAMPLE_B2 0x37
#define PACKET_BODY_LENGTH 10
#define PACKET_LENGTH (PACKET_BODY_LENGTH + 5)
#define BATTERY_MAX 12.6
#define BATTERY_MIN 10.0
// #define JOYSTICK_ENABLE
// #define BLE_ENABLE

#ifdef BLE_ENABLE
#include "gattlib.h"

#define NUS_CHARACTERISTIC_TX_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_CHARACTERISTIC_RX_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

gatt_connection_t* m_connection;

uint16_t ble_tx_handle;
uint16_t ble_rx_handle;

void ble_send();
void ble_connect();
void ble_disconnect();

void* ble_adapter;
char* ble_addr;

#define BLE_DEVICE_ID "E9:CB:02:21:5B:B7"
//#define BLE_DEVICE_ID "D6:17:B4:FA:71:9A" // Joseph board
//#define BLE_DEVICE_ID "D6:AD:A9:01:8D:D8" // George board
#define BLE_DEVICE_NAME "Quatrippel"
#define BLE_SCAN_TIMEOUT 4

#endif

extern const uint8_t crc8_table[];

extern int serial_device;
int fd_RS232;
int fd_joystick;

/**
 *	@author Georgios Giannakaras
 */
struct pcState {
	//KEYBOARD
	bool escPressed; //abort / exit

	uint8_t mode;

	bool n0Pressed;
	bool n1Pressed;
	bool n2Pressed;
	bool n3Pressed;
	bool n4Pressed;
	bool n5Pressed;
	bool n6Pressed;
	bool n7Pressed;
	bool n8Pressed;
	bool n9Pressed;

	bool aPressed; //lift up
	bool zPressed; //lift down
	uint16_t liftValue;

	bool leftPressed; //roll up
	bool rightPressed; //roll down
	uint8_t rollValue;
	bool upPressed; //pitch down
	bool downPressed; //pitch up
	uint8_t pitchValue;

	bool qPressed; //yaw down
	bool wPressed; //yaw up
	uint8_t yawValue;

	bool uPressed; //yaw control P up
	bool jPressed; //yaw control P down
	uint16_t PValue;

	bool iPressed; //roll/pitch control P1 up
	bool kkPressed; //roll/pitch control P1 down
	uint16_t P1Value;

	bool oPressed; //roll/pitch control P2 up
	bool lPressed; //roll/pitch control P2 down
	uint16_t P2Value;

	bool yPressed; //height control Pheight up
	bool hPressed; //height control Pheight down
	uint16_t PheightValue;

	bool tPressed; //height control Pheight up
	bool gPressed; //height control Pheight down
	uint8_t PheightValue2;

	//JOYSTICK
	bool jLift;
	bool jChanged;
	bool jThrottleUp; //lift up
	bool jThrottleDown; //lift down
	bool jLeft; //roll up
	bool jRight; //roll down
	bool jForward; //pitch down
	bool jBackward; //pitch up
	bool jTwistClockwise; //yaw up
	bool jTwistCounterClockwise; //yaw down
	bool jFire; //abort / exit
	uint16_t jThrottleValue;
	uint8_t jRollValue;
	uint8_t jPitchValue;
	uint8_t jYawValue;

	//TOTAL
	uint16_t tLiftValue;
	int16_t tRollValue;
	int16_t tPitchValue;
	int16_t tYawValue;
};

uint8_t DroneStatusMode;
struct pcState* pcStateGui;
struct pcState* pcState;

/**
 *	@author Georgios Giannakaras
 */
typedef struct _Widgets Widgets;
struct _Widgets {
	GtkLabel* l[26];
	GtkLevelBar* lb[4];
	GtkProgressBar* pb[1];
};

GtkBuilder *builder, *builder2;
GtkWidget *window, *dialog;
Widgets widg;
int response;

/**
 *	@author Georgios Giannakaras
 */
typedef struct {
	float batteryVolt;
	float fractionGUI;
	int batteryPercentageGUI;
} Battery;

Battery battery;

/**
 *	@author Alex Lyrakis
 */
typedef struct {
	uint16_t fcs;
	uint8_t payload[10];
	uint8_t crc;
} SRPacket;

SRPacket sPacketGUI, rPacketGUI;

FILE* Rfile;
FILE* Sfile;

FILE* CsvFile;
// #define EXTENDED_LOGGING
#ifdef EXTENDED_LOGGING
FILE* CsvFile2;
#endif

SRPacket sPacketBuffer[65535];
bool receivedACK[65535];
uint16_t motor[4];
char droneModeGUI[20];

int term_getchar_nb();
void initializations(struct pcState* pcState);
void setPacket(struct pcState* pcState, SRPacket* sPacket);
void sendPacket(SRPacket sPacket);
void logReceivePacket(SRPacket rPacket);
void receivePacket(SRPacket rPacket);
void initLogFiles();
void logSendPacket(SRPacket sPacket);
void updatePcState(struct pcState* pcState);
void initReceivedACK();
void calculateBatteryStatus();
void caluclateDroneMode(SRPacket* rPacket);
void* guiThread(void* vargp);
void on_button_safe_clicked(GtkButton* button, Widgets* widg);
void on_button_panic_clicked(GtkButton* button, Widgets* widg);
void on_button_manual_clicked(GtkButton* button, Widgets* widg);
void on_button_calibration_clicked(GtkButton* button, Widgets* widg);
void on_button_yaw_clicked(GtkButton* button, Widgets* widg);
void on_button_fullControl_clicked(GtkButton* button, Widgets* widg);
void on_button_raw_clicked(GtkButton* button, Widgets* widg);
void on_button_height_clicked(GtkButton* button, Widgets* widg);
void on_button_wireless_clicked(GtkButton* button, Widgets* widg);
void on_button_abort_clicked(GtkButton* button, Widgets* widg);
void on_button_up_clicked(GtkButton* button, Widgets* widg);
void on_button_down_clicked(GtkButton* button, Widgets* widg);
void printPcStatusGUI(SRPacket* sPacket);
void printMotorStatusGUI(SRPacket* rPacket);
void printDroneStatusGUI(SRPacket* rPacket);
void printModeGUI(SRPacket* rPacket);
void printBatteryStatusGUI();
void printControllersGUI(SRPacket* sPacket);
void writePing();
uint64_t getMicrotime();

bool emptiedBuffer;

#endif
