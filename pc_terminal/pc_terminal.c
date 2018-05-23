/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <pthread.h>

#include "pc_terminal.h"
#include "pcqueue.h"
#include "joystick.h"
#include "keyboard.h"
#include "rs232.h"
//#include "commLog.h"
//#include <sys/time.h>

/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */

const uint8_t crc8_table[] = {
    0xea, 0xd4, 0x96, 0xa8, 0x12, 0x2c, 0x6e, 0x50, 0x7f, 0x41, 0x03, 0x3d,
    0x87, 0xb9, 0xfb, 0xc5, 0xa5, 0x9b, 0xd9, 0xe7, 0x5d, 0x63, 0x21, 0x1f,
    0x30, 0x0e, 0x4c, 0x72, 0xc8, 0xf6, 0xb4, 0x8a, 0x74, 0x4a, 0x08, 0x36,
    0x8c, 0xb2, 0xf0, 0xce, 0xe1, 0xdf, 0x9d, 0xa3, 0x19, 0x27, 0x65, 0x5b,
    0x3b, 0x05, 0x47, 0x79, 0xc3, 0xfd, 0xbf, 0x81, 0xae, 0x90, 0xd2, 0xec,
    0x56, 0x68, 0x2a, 0x14, 0xb3, 0x8d, 0xcf, 0xf1, 0x4b, 0x75, 0x37, 0x09,
    0x26, 0x18, 0x5a, 0x64, 0xde, 0xe0, 0xa2, 0x9c, 0xfc, 0xc2, 0x80, 0xbe,
    0x04, 0x3a, 0x78, 0x46, 0x69, 0x57, 0x15, 0x2b, 0x91, 0xaf, 0xed, 0xd3,
    0x2d, 0x13, 0x51, 0x6f, 0xd5, 0xeb, 0xa9, 0x97, 0xb8, 0x86, 0xc4, 0xfa,
    0x40, 0x7e, 0x3c, 0x02, 0x62, 0x5c, 0x1e, 0x20, 0x9a, 0xa4, 0xe6, 0xd8,
    0xf7, 0xc9, 0x8b, 0xb5, 0x0f, 0x31, 0x73, 0x4d, 0x58, 0x66, 0x24, 0x1a,
    0xa0, 0x9e, 0xdc, 0xe2, 0xcd, 0xf3, 0xb1, 0x8f, 0x35, 0x0b, 0x49, 0x77,
    0x17, 0x29, 0x6b, 0x55, 0xef, 0xd1, 0x93, 0xad, 0x82, 0xbc, 0xfe, 0xc0,
    0x7a, 0x44, 0x06, 0x38, 0xc6, 0xf8, 0xba, 0x84, 0x3e, 0x00, 0x42, 0x7c,
    0x53, 0x6d, 0x2f, 0x11, 0xab, 0x95, 0xd7, 0xe9, 0x89, 0xb7, 0xf5, 0xcb,
    0x71, 0x4f, 0x0d, 0x33, 0x1c, 0x22, 0x60, 0x5e, 0xe4, 0xda, 0x98, 0xa6,
    0x01, 0x3f, 0x7d, 0x43, 0xf9, 0xc7, 0x85, 0xbb, 0x94, 0xaa, 0xe8, 0xd6,
    0x6c, 0x52, 0x10, 0x2e, 0x4e, 0x70, 0x32, 0x0c, 0xb6, 0x88, 0xca, 0xf4,
    0xdb, 0xe5, 0xa7, 0x99, 0x23, 0x1d, 0x5f, 0x61, 0x9f, 0xa1, 0xe3, 0xdd,
    0x67, 0x59, 0x1b, 0x25, 0x0a, 0x34, 0x76, 0x48, 0xf2, 0xcc, 0x8e, 0xb0,
    0xd0, 0xee, 0xac, 0x92, 0x28, 0x16, 0x54, 0x6a, 0x45, 0x7b, 0x39, 0x07,
    0xbd, 0x83, 0xc1, 0xff};

struct termios savetty;
int serial_device = 0;

#include <sys/time.h>


// @Author Alex Lyrakis
void setPacket(struct pcState *pcState, SRPacket *sPacket){
	// Set packet type
	static uint16_t sPacketCounter = 0;
	sPacket->fcs = sPacketCounter;
	sPacket->payload[0] = 0; //Default type
	if (setModeAttempt(pcState))
		sPacket->payload[0] = 5;
	if (setControlAttempt(pcState) || pcState->jChanged)
		sPacket->payload[0] = 3;
	if (setPAttempt(pcState))
		sPacket->payload[0] = 9;

	// Set payload
	switch (sPacket->payload[0]) {
		case 5:
			//here we must send the packet
			sPacket->payload[1] = pcState->mode;
			for (int i = 2; i < PACKET_BODY_LENGTH; i++){
				sPacket->payload[i] = 0;
			}
			break;
		case 3:
			if (pcState->escPressed || pcState->jFire){
				sPacket->payload[1] = 0x80;	// abort byte
			} else {
				sPacket->payload[1] = 0;  // else zero
			}
			sPacket->payload[2] = (uint8_t) pcState->tRollValue;
			sPacket->payload[3] = (uint8_t) pcState->tPitchValue;
			sPacket->payload[4] = (uint8_t) pcState->tYawValue;
			sPacket->payload[5] =(uint8_t) (pcState->tLiftValue >> 8);
			sPacket->payload[6] =(uint8_t) (pcState->tLiftValue & 0xFF);
			for (int i = 7; i < PACKET_BODY_LENGTH; i++){
				sPacket->payload[i] = 0; // null bytes
			}
			break;
		case 9:
			sPacket->payload[1] = (pcState->P1Value >> 8);
			sPacket->payload[2] = (pcState->P1Value & 0xFF);
			sPacket->payload[3] = (pcState->P2Value >> 8);
			sPacket->payload[4] = (pcState->P2Value & 0xFF);
			sPacket->payload[5] = (pcState->PValue >> 8);
			sPacket->payload[6] = (pcState->PValue & 0xFF);
			for (int i = 7; i < PACKET_BODY_LENGTH; i++){
				sPacket->payload[i] = 0; // null bytes
			}
	}
	// // Set crc
	// sPacket->crc = 0x00;
	// sPacket->crc = crc8_table[sPacket->crc ^ ((uint8_t) (sPacket->fcs >> 8))];
	// sPacket->crc = crc8_table[sPacket->crc ^ ((uint8_t) (sPacket->fcs & 0xFF))];
	// for (int i=0; i<10; i++) {
	// 	sPacket->crc = crc8_table[sPacket->crc ^ sPacket->payload[i]];
	// }
	// Increase packet counter
	if (sPacketCounter == 0xFFFF)
		sPacketCounter = 0;
	else
		sPacketCounter++;
}

//@Author Alex Lyrakis
void sendPacket(SRPacket sPacket){
	rs232_putchar(0x13);
	rs232_putchar(0x37);
	rs232_putchar(sPacket.fcs >> 8);
	rs232_putchar(sPacket.fcs & 0xFF);
	for (int i=0; i<10; i++){
		rs232_putchar(sPacket.payload[i]);
	}
	sPacket.crc = 0x00;
	sPacket.crc = crc8_table[sPacket.crc ^ ((uint8_t) (sPacket.fcs >> 8))];
	sPacket.crc = crc8_table[sPacket.crc ^ ((uint8_t) (sPacket.fcs & 0xFF))];
	for (int i=0; i<10; i++) {
		sPacket.crc = crc8_table[sPacket.crc ^ sPacket.payload[i]];
	}
	rs232_putchar(sPacket.crc);
}

//@Author George Giannakaras
void receivePacket(SRPacket rPacket){
	uint8_t crc = 0x00, crcTemp;
	bool foundPacket = false;

	while(pcReQueue.count >= PACKET_LENGTH && foundPacket == false){
		if(queuePeekpc(&pcReQueue, 0) == PREAMPLE_B1 && queuePeekpc(&pcReQueue, 1) == PREAMPLE_B2){
			crc = 0x00;
			for(uint16_t j = 2; j < (PACKET_LENGTH - 1); j++){
				// printf("Byte(%hu) %hhu ", j, queuePeekpc(&pcReQueue, j));
				crc = crc8_table[crc ^ queuePeekpc(&pcReQueue, j)];
			}
			crcTemp = queuePeekpc(&pcReQueue, PACKET_LENGTH - 1);
			if (crc == crcTemp) {
				foundPacket = true;
				//discard preamble bytes
				dequeuepc(&pcReQueue);
				dequeuepc(&pcReQueue);

				rPacket.fcs = dequeuepc(&pcReQueue) << 8;
				rPacket.fcs = rPacket.fcs | dequeuepc(&pcReQueue);
				for(int j = 0; j < PACKET_BODY_LENGTH; j++){
					rPacket.payload[j] = dequeuepc(&pcReQueue);
				}
				//discard crc
				dequeuepc(&pcReQueue);
				// printf("Received %hhu\n", rPacket->payload[0]);
				rPacketGUI = rPacket;
				switch(rPacket.payload[0]) {
					case 2:
						logReceivePacket(rPacket);
						break;
					case 7:
						logReceivePacket(rPacket);
						break;
					case 10:
						logReceivePacket(rPacket);
						break;
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
						logReceivePacket(rPacket);
						break;
					case 11:
						//Ack
						receivedACK[rPacket.fcs] = true;
						break;
				}
			} else {
				printf("Invalid CRC, expected:%hhu but got:%hhu\n", crcTemp, crc);
				dequeuepc(&pcReQueue);
			}
		}
		else{
			dequeuepc(&pcReQueue);
		}
	}
}


//@Author Alex Lyrakis
void initLogFiles(){
	Rfile = fopen("logReceivePacket.txt", "a");
	Sfile = fopen("logSendPackets.txt", "a");
	system("cat /dev/null > logSendPackets.txt");
	system("cat /dev/null > logReceivePacket.txt");
	if (Rfile == NULL)
	{
	    printf("Error opening logReceivePacket.txt file!\n");
	    exit(1);
	}
	if (Sfile == NULL)
	{
	    printf("Error opening logSendPackets.txt file!\n");
	    exit(1);
	}
}

//@Author George Giannakaras
void logReceivePacket(SRPacket rPacket){
	uint32_t val, val2;
	uint64_t val3;
	static int counter = 0;

	counter++;
	switch(rPacket.payload[0]){
		case 2:
			emptiedBuffer = true;
      		battery.batteryVolt = (((float) rPacket.payload[2]) * 7 / 100) + 1.2;
			printf("System time: %hhu | Packet number: %hu | Type: %hhu | Mode: %hhu | Battery: %f | Roll: %hhu | Pitch: %hhu | Height: %hhu\n",
				rPacket.payload[6], rPacket.fcs, rPacket.payload[0], rPacket.payload[1], battery.batteryVolt, rPacket.payload[3], rPacket.payload[4], rPacket.payload[5]);
			fprintf(Rfile, "System time: %hu | Packet number: %hu | Type: %hhu | Mode: %hu | Battery: %hu | Roll: %hu | Pitch: %hu | Height: %hu\n",
				rPacket.payload[6], rPacket.fcs, rPacket.payload[0], rPacket.payload[1], rPacket.payload[2], rPacket.payload[3],
				rPacket.payload[4], rPacket.payload[5]);
			#ifdef GUIACTIVATED
				calculateBatteryStatus();
				g_idle_add ((GSourceFunc) printBatteryStatusGUI, NULL);				
				g_idle_add ((GSourceFunc) printDroneStatusGUI, &rPacket);
				//printDroneStatusGUI(&rPacket);
				caluclateDroneMode(&rPacket);
				g_idle_add ((GSourceFunc) printModeGUI, &rPacket);
				//printModeGUI(&rPacket);
			#endif
			break;
		case 7:
			printf("Type: %hhu | ERROR: %hhu\n", rPacket.payload[0], rPacket.payload[1]);
			// fprintf(Rfile, "Type: %hhu | ERROR: %hu\n", rPacket.payload[0], rPacket.payload[1]);
			break;
		case 10:
			motor[0] = (uint16_t) rPacket.payload[1] << 8 | (uint16_t)rPacket.payload[2];
			motor[1] = (uint16_t)rPacket.payload[3] << 8 | (uint16_t)rPacket.payload[4];
			motor[2] = (uint16_t)rPacket.payload[5] << 8 | (uint16_t)rPacket.payload[6];
			motor[3] = (uint16_t)rPacket.payload[7] << 8 | (uint16_t)rPacket.payload[8];
			#ifdef GUIACTIVATED
				g_idle_add ((GSourceFunc) printMotorStatusGUI, &rPacket);
				//printMotorStatusGUI(&rPacket);
			#endif
			printf("Packet number: %hu | Type: %hhu | Motor1: %hu | Motor2: %hu | Motor3: %hu | Motor4: %hu\n",
				rPacket.fcs, rPacket.payload[0], motor[0], motor[1], motor[2], motor[3]);
			fprintf(Rfile, "Packet number: %hu | Type: %hhu | Motor1: %hu | Motor2: %hu | Motor3: %hu | Motor4: %hu\n",
				rPacket.fcs, rPacket.payload[0], motor[0], motor[1], motor[2], motor[3]);
			break;
		case 12:
			printf("Receive ping\n");
			writePing();
			break;
		case 13:
			val3 = ((uint64_t)rPacket.payload[1] << 56) | 
				((uint64_t)rPacket.payload[2] << 48) | 
				((uint64_t)rPacket.payload[3] << 40) | 
				((uint64_t)rPacket.payload[4] << 32) | 
				((uint64_t)rPacket.payload[5] << 24) | 
				((uint64_t)rPacket.payload[6] << 16) | 
				((uint64_t)rPacket.payload[7] << 8) | 
				((uint64_t)rPacket.payload[8]); 
			printf("Receive pong %lu\n", getMicrotime() - val3);
			break;
		case 14:
			val = (uint32_t)rPacket.payload[2] << 24 | (uint32_t)rPacket.payload[3] << 16 | (uint32_t)rPacket.payload[4] << 8 | (uint32_t)rPacket.payload[5];
			val2 = (uint32_t)rPacket.payload[6] << 24 | (uint32_t)rPacket.payload[7] << 16 | (uint32_t)rPacket.payload[8] << 8 | (uint32_t)rPacket.payload[9];
			switch(rPacket.payload[1]) {
				case 1:
					printf("Loop time=%u, last=%u\n", val, val2);
					break;
				case 2:
					printf("Packet Loop time=%u, last=%u\n", val, val2);
					break;
				case 3:
					printf("Control Loop time=%u, last=%u\n", val, val2);
					break;
				case 4:
					printf("Application Loop time=%u, last=%u\n", val, val2);
					break;
				case 5:
					printf("Sensor Loop time=%u, last=%u\n", val, val2);
					break;
			}
			break;
		case 15:
			printf("[RAW]phi=%u, theta=%u, psi= %u\n",
				(((uint16_t) rPacket.payload[1]) << 8) | ((uint16_t) rPacket.payload[2]),
				(((uint16_t) rPacket.payload[3]) << 8) | ((uint16_t) rPacket.payload[4]),
				(((uint16_t) rPacket.payload[5]) << 8) | ((uint16_t) rPacket.payload[6])
			);
			break;
		case 16:
			printf("[RAW]sp=%u, sq=%u, sr= %u\n",
				(((uint16_t) rPacket.payload[1]) << 8) | ((uint16_t) rPacket.payload[2]),
				(((uint16_t) rPacket.payload[3]) << 8) | ((uint16_t) rPacket.payload[4]),
				(((uint16_t) rPacket.payload[5]) << 8) | ((uint16_t) rPacket.payload[6])
			);
			break;
	}
}


//@Author Alex Lyrakis
void logSendPacket(SRPacket sPacket){
	switch(sPacket.payload[0]){
		case 3:
			fprintf(Sfile, "Packet number: %hu | Type: %hhu | Abort: %hhu | Roll: %hhu | Pitch: %hhu | Yaw: %hhu | HeightByte1: %hhu | HeightByte0: %hhu",
						sPacket.fcs, sPacket.payload[0], sPacket.payload[1], sPacket.payload[2], sPacket.payload[3], sPacket.payload[4], sPacket.payload[5], sPacket.payload[6]);
			fprintf(Sfile, " | crc: %hhu \n", sPacket.crc);
			#ifdef GUIACTIVATED
				g_idle_add ((GSourceFunc) printPcStatusGUI, &sPacket);
			#endif
				//printPcStatusGUI(&sPacket);
			break;
		case 5:
			// fprintf(Sfile, "Packet number: %hu | Type: %hhu | Mode: %hhu",
						// sPacket.fcs, sPacket.payload[0], sPacket.payload[1]);
			// fprintf(Sfile, " | crc: %hhu \n", sPacket.crc);
			break;
		case 9:
			// fprintf(Sfile, "Packet number: %hu | Type: %hhu | P_rollByte1: %hhu | P_rollByte0: %hhu | P_pitchByte1: %hhu | P_pitchByte0: %hhu | P_yawByte1: %hhu | P_yawByte0: %hhu",
						// sPacket.fcs, sPacket.payload[0], sPacket.payload[1], sPacket.payload[2], sPacket.payload[3], sPacket.payload[4], sPacket.payload[5], sPacket.payload[6]);
			// fprintf(Sfile, " | crc: %hhu \n", sPacket.crc);
			break;
	}
}

//@Author Alex Lyrakis
void updatePcState(struct pcState *pcState){

	pcState->tLiftValue = pcState->liftValue + pcState->jThrottleValue;
	pcState->tRollValue = pcState->rollValue + pcState->jRollValue - 90;
	pcState->tPitchValue = pcState->pitchValue + pcState->jPitchValue - 90;
	pcState->tYawValue = pcState->yawValue + pcState->jYawValue - 90;
	if (pcState->tRollValue < 0) {
		pcState->tRollValue = 0;
	}
	else if (pcState->tRollValue > 180) {
    pcState->tRollValue = 180;
	}
    if (pcState->tPitchValue < 0) {
    pcState->tPitchValue = 0;
    }
    else if (pcState->tPitchValue > 180) {
    pcState->tPitchValue = 180;
    }
    if (pcState->tYawValue < 0) {
      pcState->tYawValue = 0;
    }
    else if (pcState->tYawValue > 180) {
      pcState->tYawValue = 180;
    }
	if (pcState->tLiftValue > 1000){
		pcState->tLiftValue = 1000;
	}
	/*
	if( (pcState->tLiftValue < 200) && (pcState->liftValue >= 10) ){	// Motors start moving after 200 rpm
		pcState->tLiftValue += 190;
	}
	*/
}

void initReceivedACK(){
	for(int i = 0; i < 65535; i++){
		receivedACK[i] = false;
	}
}




/**
 * Returns the current time in microseconds.
 */
uint64_t getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

void	term_initio()
{
	struct termios tty;

	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);

	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;

	tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s)
{
	// fprintf(stderr,"%s",s);
}

void	term_putchar(char c)
{
	putc(c,stderr);
}

int	term_getchar_nb()
{
        static unsigned char 	line [2];

        if (read(0,line,1)) // note: destructive read
        		return (int) line[0];

        return -1;
}

int	term_getchar()
{
        int    c;

        while ((c = term_getchar_nb()) == -1)
                ;
        return c;
}

/*------------------------------------------------------------
 * Serial I/O
 * 8 bits, 1 stopbit, no parity,
 * 115,200 baud
 *------------------------------------------------------------
 */



void writePing() {
	SRPacket rPacket;
	rPacket.fcs = 0;
	uint64_t time = getMicrotime();
	rPacket.payload[0] = 12;
	rPacket.payload[1] = (time >> 56) & 0xFF;
	rPacket.payload[2] = (time >> 48) & 0xFF;
	rPacket.payload[3] = (time >> 40) & 0xFF;
	rPacket.payload[4] = (time >> 32) & 0xFF;
	rPacket.payload[5] = (time >> 24) & 0xFF;
	rPacket.payload[6] = (time >> 16) & 0xFF;
	rPacket.payload[7] = (time >> 8) & 0xFF;
	rPacket.payload[8] = (time) & 0xFF;

	printf("%hhu", rPacket.payload[1]);
	
	sendPacket(rPacket);
}

//@Author Georgios Giannakaras
void calculateBatteryStatus()
{
		float temp_battery;
		float battery_range = BATTERY_MAX - BATTERY_MIN;
		int battery_final;

		temp_battery = (battery.batteryVolt - BATTERY_MIN);
		battery_range = battery_range / 100;
		temp_battery = temp_battery / battery_range;
		battery_final = (int) temp_battery;
		battery.fractionGUI = temp_battery / 100;
		if (battery_final < 0)
		{
			battery_final = 0;
			battery.fractionGUI = 0;
		}
		else if(battery_final > 100){
			battery_final = 100;
			battery.fractionGUI = 1;
		}
		battery.batteryPercentageGUI = battery_final;
		
}

//@Author Georgios Giannakaras
void printBatteryStatusGUI(){
	char guiText[30];

	gtk_progress_bar_set_fraction (widg.pb[0], battery.fractionGUI);
	sprintf(guiText, "%d%%", battery.batteryPercentageGUI);
	gtk_progress_bar_set_text (widg.pb[0], guiText);

	sprintf(guiText, "%0.3f V", battery.batteryVolt);
	gtk_label_set_label(widg.l[5], guiText);
}

//@Author Georgios Giannakaras
void printMotorStatusGUI(SRPacket *rPacket){
	char guiText[30];

	for (int i = 0; i < 4; ++i)
	{
		sprintf(guiText, "%hu RPM", motor[i]);
		gtk_label_set_label(widg.l[i], guiText);
		gtk_level_bar_set_value (widg.lb[i], motor[i]);
	}
}

//@Author Georgios Giannakaras
void caluclateDroneMode(SRPacket *rPacket){
	switch(rPacket->payload[1]){
		case 0:
			sprintf(droneModeGUI, "Safe");
			break;
		case 1:
			sprintf(droneModeGUI, "Panic");
			break;
		case 2:
			sprintf(droneModeGUI, "Manual");
			break;
		case 3:
			sprintf(droneModeGUI, "Calibration");
			break;
		case 4:
			sprintf(droneModeGUI, "Yaw");
			break;
		case 5:
			sprintf(droneModeGUI, "Full Control");
			break;
		case 6:
			sprintf(droneModeGUI, "Raw");
			break;
		case 7:
			sprintf(droneModeGUI, "Height");
			break;
		case 8:
			sprintf(droneModeGUI, "Wireless");
			break;
	}
}

//@Author Georgios Giannakaras
void printModeGUI(SRPacket *rPacket){

	gtk_label_set_label(widg.l[4], droneModeGUI);
}

//@Author Georgios Giannakaras
void printDroneStatusGUI(SRPacket *rPacket){
	char guiText[30];

	for (int i = 3; i < 6; ++i)
	{
		sprintf(guiText, "%hhu", rPacketGUI.payload[i]);
		gtk_label_set_label(widg.l[i+3], guiText);
	}
}

//@Author Georgios Giannakaras
void printPcStatusGUI(SRPacket *sPacket){
	char guiText[20];
	uint16_t lift;

	//Print pc state to GUI
	for (int i = 2; i < 5; ++i)
	{
		sprintf(guiText, "%hhu", sPacketGUI.payload[i]);
		gtk_label_set_label(widg.l[i+7], guiText);
	}
	lift = sPacketGUI.payload[5] << 8 | sPacketGUI.payload[6];
	sprintf(guiText, "%hu", lift);
	gtk_label_set_label(widg.l[12], guiText);

	//Print keyboard to GUI
	sprintf(guiText, "%hhu", pcStateGui->rollValue);
	gtk_label_set_label(widg.l[13], guiText);
	sprintf(guiText, "%hhu", pcStateGui->pitchValue);
	gtk_label_set_label(widg.l[14], guiText);
	sprintf(guiText, "%hhu", pcStateGui->yawValue);
	gtk_label_set_label(widg.l[15], guiText);
	sprintf(guiText, "%hu", pcStateGui->liftValue);
	gtk_label_set_label(widg.l[16], guiText);

	//Print joystick to GUI
	sprintf(guiText, "%hhu", pcStateGui->jRollValue);
	gtk_label_set_label(widg.l[17], guiText);
	sprintf(guiText, "%hhu", pcStateGui->jPitchValue);
	gtk_label_set_label(widg.l[18], guiText);
	sprintf(guiText, "%hhu", pcStateGui->jYawValue);
	gtk_label_set_label(widg.l[19], guiText);
	sprintf(guiText, "%hu", pcStateGui->jThrottleValue);
	gtk_label_set_label(widg.l[20], guiText);
}



//@Author Georgios Giannakaras
void initializations(struct pcState *pcState){
	char c;

	term_puts("\nTerminal program - Embedded Real-Time Systems\n");
	emptiedBuffer = false;
	term_initio();
	term_puts("Initialized termios...\n");
	rs232_open();
	term_puts("Initialized rs232...\n");
	#ifdef JOYSTICK_ENABLE
	openJoystick();
	#endif
	term_puts("Initialized joystick...\n");

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	term_puts("Empty usb buffer\n");
	while ((c = rs232_getchar_nb()) != -1){
		fputc(c,stderr);
	}
	printf("\n");
	initLogFiles();
	initReceivedACK();
	initPcState(pcState);
	resetPcState(pcState); // Reset values and State of PC side.
	init_queuepc(&pcReQueue);
}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
#define COMMUNICATION_MIN_DELAY_US (11 * 1000) 
#define COMMUNICATION_PING_INTERVAL_US (1000 * 1000) 
int main(int argc, char **argv)
{
	pcState = (struct pcState*) calloc(1, sizeof(struct pcState));
	SRPacket sPacket;
	SRPacket rPacket;
	bool bufferCleared = false;
	int c;
	
	long timeLastPacket = 0;
	long timeLastPing = getMicrotime();
	
	initializations(pcState);
	pcStateGui = pcState;
	
	#ifdef GUIACTIVATED
	pthread_t guithread;
	pthread_create(&guithread, NULL, guiThread, NULL);
	#endif

	//send & receive
	while(1) {
		if ((c = term_getchar_nb()) != -1)	// Read from keyboard and store in fd_RS232
		{
			checkInput(c, pcState);
		}

		// Read from joystic and update pcState
   	 	
		#ifdef JOYSTICK_ENABLE
		checkJoystick(pcState);
		#endif

		// Read from fd_RS232
		if ((c = rs232_getchar_nb()) != -1){
			if(c == 0x13){
				bufferCleared = true;
			}
			if(bufferCleared) {
				enqueuepc(&pcReQueue, (uint8_t) c);
			}
			if(pcReQueue.count >= PACKET_LENGTH) {
				receivePacket(rPacket);
			}
		}

		if ((getMicrotime() - timeLastPacket) >= COMMUNICATION_MIN_DELAY_US){
			if ((getMicrotime() - timeLastPing) >= COMMUNICATION_PING_INTERVAL_US) {
				timeLastPing = getMicrotime();
				writePing();
			} else if (sthPressed(pcState) || pcState->jChanged){
				updatePcState(pcState);
				
				setPacket(pcState, &sPacket);
				sendPacket(sPacket);
				sPacketGUI = sPacket;
				logSendPacket(sPacket);
				//if (pcState->escPressed)
				//	break;
				resetPcState(pcState);
				sPacketBuffer[sPacket.fcs] = sPacket;
			}

			//TBD: Based on our pcState and protocol we have to put a sequence of bytes using rs232_putchar(c);
			//		After we have to reset the pcState.
			
			timeLastPacket = getMicrotime();
		}

		/*
		endLoop = clock();
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
		loopTime = (endLoop - beginLoop))/CLOCKS_PER_SEC;
		printf("%" PRIu64 "\n", delta_us);
		printf("Time of loop is : %Lf \n", loopTime);
		*/
	}

	free(pcState);
	term_exitio();
	rs232_close();
	fclose(Rfile);
	fclose(Sfile);
	term_puts("\n<exit>\n");

	return 0;
}
