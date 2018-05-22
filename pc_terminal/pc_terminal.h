#include <stdint.h>
#include <stdbool.h>

// #define GUIACTIVATED

#ifdef GUIACTIVATED
#include <gtk/gtk.h>
#endif

#ifndef PC_TERMINAL_H_
#define PC_TERMINAL_H_


#include "./communication.h"

#define PREAMPLE_B1 0x13
#define PREAMPLE_B2 0x37
#define PACKET_BODY_LENGTH 10
#define PACKET_LENGTH (PACKET_BODY_LENGTH + 5)



#define BATTERY_MAX 12.6
#define BATTERY_MIN 10.5
// #define JOYSTICK_ENABLE 1


extern const uint8_t crc8_table[];

extern int serial_device;
int fd_RS232;
int fd_joystick;
// @Author: George Giannakaras
struct pcState{
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

	bool aPressed;	//lift up
	bool zPressed;	//lift down
	uint16_t liftValue;

	bool leftPressed;	//roll up
	bool rightPressed;	//roll down
	uint8_t rollValue;
	bool upPressed;		//pitch down
	bool downPressed;	//pitch up
	uint8_t pitchValue;

	bool qPressed;	//yaw down
	bool wPressed;	//yaw up
	uint8_t yawValue;

	bool uPressed;	//yaw control P up
	bool jPressed;	//yaw control P down
	uint16_t PValue;

	bool iPressed;	//roll/pitch control P1 up
	bool kkPressed;	//roll/pitch control P1 down
	uint16_t P1Value;

	bool oPressed;	//roll/pitch control P2 up
	bool lPressed;	//roll/pitch control P2 down
	uint16_t P2Value;

	//JOYSTICK
	bool jChanged;
	bool jThrottleUp;	//lift up
	bool jThrottleDown;	//lift down
	bool jLeft;		//roll up
	bool jRight;	//roll down
	bool jForward;	//pitch down
	bool jBackward;	//pitch up
	bool jTwistClockwise;			//yaw up
	bool jTwistCounterClockwise;	//yaw down
	bool jFire;	//abort / exit
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

struct pcState *pcStateGui;
struct pcState *pcState;

#ifdef GUIACTIVATED
//@Author Georgios Giannakaras
typedef struct _Widgets Widgets;
struct _Widgets
{
	GtkLabel *l[21];
	GtkLevelBar *lb[4];
	GtkProgressBar *pb[1];
};

GtkBuilder *builder;
GtkWidget  *window;
Widgets widg;
#endif

SRPacket sPacketGUI, rPacketGUI;

FILE *Rfile;
FILE *Sfile;

void initializations(struct pcState *pcState);
void setPacket(struct pcState *pcState, SRPacket *sPacket);
void sendPacket(SRPacket sPacket);
void logReceivePacket(SRPacket rPacket);
void receivePacket(SRPacket rPacket);
void initLogFiles();
void logSendPacket(SRPacket sPacket);
void updatePcState(struct pcState *pcState);

#ifdef GUIACTIVATED
void calculateBatteryStatus(float battery);
void *guiThread(void *vargp);
void on_button_safe_clicked(GtkButton *button, Widgets *widg);
void on_button_panic_clicked(GtkButton *button, Widgets *widg);
void on_button_manual_clicked(GtkButton *button, Widgets *widg);
void on_button_calibration_clicked(GtkButton *button, Widgets *widg);
void on_button_yaw_clicked(GtkButton *button, Widgets *widg);
void on_button_fullControl_clicked(GtkButton *button, Widgets *widg);
void on_button_raw_clicked(GtkButton *button, Widgets *widg);
void on_button_height_clicked(GtkButton *button, Widgets *widg);
void on_button_wireless_clicked(GtkButton *button, Widgets *widg);
void on_button_abort_clicked(GtkButton *button, Widgets *widg);
void on_button_up_clicked(GtkButton *button, Widgets *widg);
void on_button_down_clicked(GtkButton *button, Widgets *widg);
void printPcStatusGUI(SRPacket *sPacket);
void printMotorStatusGUI(SRPacket *rPacket);
void printDroneStatusGUI(SRPacket *rPacket);
void printModeGUI(SRPacket *rPacket);
#endif 

uint64_t getMicrotime();

bool emptiedBuffer;




#endif
