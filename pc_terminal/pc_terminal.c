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

#include "pc_terminal.h"
#include "pcqueue.h"
#include "joystick.h"



/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */


static uint8_t const crc8_table[] = {
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

struct termios 	savetty;

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
	fprintf(stderr,"%s",s);
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
int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
  	char *name;
  	int result;
  	struct termios	tty;

    fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);  // Hardcode your serial port here, or request it as an argument at runtime

	assert(fd_RS232>=0);

  	result = isatty(fd_RS232);
  	assert(result == 1);

  	name = ttyname(fd_RS232);
  	assert(name != 0);

  	result = tcgetattr(fd_RS232, &tty);
	assert(result == 0);

	tty.c_iflag = IGNBRK; /* ignore break condition */
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
	tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 1; // added timeout

	tty.c_iflag &= ~(IXON|IXOFF|IXANY);

	result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

	tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


void 	rs232_close(void)
{
  	int 	result;

  	result = close(fd_RS232);
  	assert (result==0);
}


int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(fd_RS232, &c, 1);

	if (result == 0)
		return -1;

	else
	{
		assert(result == 1);
		return (int) c;
	}
}


int 	rs232_getchar()
{
	int 	c;

	while ((c = rs232_getchar_nb()) == -1)
		;
	return c;
}


int 	rs232_putchar(char c)
{
	int result;

	do {
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);

	assert(result == 1);
	return result;
}


// @Author: Alex Lyrakis
void resetPcState(struct pcState *pcState){
	pcState->escPressed = false;
	pcState->n0Pressed = false;
	pcState->n1Pressed = false;
	pcState->n2Pressed = false;
	pcState->n3Pressed = false;
	pcState->n4Pressed = false;
	pcState->n5Pressed = false;
	pcState->n6Pressed = false;
	pcState->n7Pressed = false;
	pcState->n8Pressed = false;

	pcState->upPressed = false;
	pcState->downPressed = false;
	pcState->leftPressed = false;
	pcState->rightPressed = false;

	pcState->aPressed = false;
	pcState->zPressed = false;
	pcState->qPressed = false;
	pcState->wPressed = false;
	pcState->uPressed = false;
	pcState->jPressed = false;
	pcState->iPressed = false;
	pcState->kkPressed = false;
	pcState->oPressed = false;
	pcState->lPressed = false;


	//JOYSTIC
	pcState->jChanged = false;
	pcState->jThrottleUp = false;
	pcState->jThrottleDown = false;
	pcState->jLeft = false;
	pcState->jRight = false;
	pcState->jForward = false;
	pcState->jBackward = false;
	pcState->jTwistClockwise = false;
	pcState->jTwistCounterClockwise = false;
	pcState->jFire = false;
}


// @Author: George Giannakaras
void initPcState(struct pcState *pcState){
  pcState->liftValue = 0;
  pcState->rollValue = 90;
  pcState->pitchValue = 90;
  pcState->yawValue = 90;
  pcState->PValue = 0;
  pcState->P1Value = 0;
  pcState->P2Value = 0;

  //JOYSTIC
  pcState->jThrottleValue = 0;
  pcState->jRollValue = 90;
  pcState->jPitchValue = 90;
  pcState->jYawValue = 90;

  // Probably initialization of keyboard and joystic values have to be switched

  //TOTAL
  pcState->tLiftValue = 0;
  pcState->tRollValue = 90;
  pcState->tPitchValue = 90;
  pcState->tYawValue = 90;
}

// @Author: Alex Lyrakis
void checkInput(char c, struct pcState *pcState)
{
	switch (c)
	{
		case 27:
			pcState->escPressed = true;
			c = term_getchar_nb();
			if (c == 91){					// If we have one 'Esc' and '[' in a row, that means it is probably an arrow command.
				c = term_getchar_nb();
				switch (c)
				{
					case 'A':
						pcState->escPressed = false;
						if (pcState->pitchValue > 0){
							pcState->upPressed = true;
							pcState->pitchValue -= 1;
						}

						break;
					case 'B':
						pcState->escPressed = false;
						if (pcState->pitchValue < 180){
							pcState->downPressed = true;
							pcState->pitchValue += 1;
						}

						break;
					case 'C':
						pcState->escPressed = false;
						if (pcState->rollValue > 0){
							pcState->rightPressed = true;
							pcState->rollValue -= 1;
						}

						break;
					case 'D':
						pcState->escPressed = false;
						if (pcState->rollValue < 180){
							pcState->leftPressed = true;
							pcState->rollValue += 1;
						}
						break;
				}
			}
			break;
		case '0':
			pcState->n0Pressed = true;
			pcState->mode = 0;
			break;
		case '1':
			pcState->n1Pressed = true;
			pcState->mode = 1;
			break;
		case '2':
			pcState->n2Pressed = true;
			pcState->mode = 2;
			break;
		case '3':
			pcState->n3Pressed = true;
			pcState->mode = 3;
			break;
		case '4':
			pcState->n4Pressed = true;
			pcState->mode = 4;
			break;
		case '5':
			pcState->n5Pressed = true;
			pcState->mode = 5;
			break;
		case '6':
			pcState->n6Pressed = true;
			pcState->mode = 6;
			break;
		case '7':
			pcState->n7Pressed = true;
			pcState->mode = 7;
			break;
		case '8':
			pcState->n8Pressed = true;
			pcState->mode = 8;
			break;
		case 'a':
			if (pcState->liftValue <=1000){
				pcState->liftValue +=10;
				pcState->aPressed = true;
      }
			break;
		case 'z':
			if (pcState->liftValue >10){
				pcState->zPressed = true;
				pcState->liftValue -=10;
			}
			break;
		case 'q':
			if (pcState->yawValue > 10){
				pcState->qPressed = true;
				pcState->yawValue -= 10;
			}
			break;
		case 'w':
			if (pcState->yawValue < 180){
				pcState->wPressed = true;
				pcState->yawValue += 10;
			}
			break;
		case 'u':
			if (pcState->PValue < 1000){
				pcState->uPressed = true;
				pcState->PValue += 10;
			}

			break;
		case 'j':
			if (pcState->PValue > 10){
				pcState->jPressed = true;
				pcState->PValue -= 10;
			}
			break;
		case 'i':
			if (pcState->P1Value < 1000){
				pcState->iPressed = true;
				pcState->P1Value += 10;
			}
			break;
		case 'k':
			if (pcState->P1Value > 10){
				pcState->kkPressed = true;
				pcState->P1Value -= 10;
			}
			break;
		case 'o':
			if (pcState->P2Value < 1000){
				pcState->oPressed = true;
				pcState->P2Value += 10;
			}
			break;
		case 'l':
			if (pcState->P2Value > 10){
				pcState->lPressed = true;
				pcState->P2Value -= 10;
			}
			break;
	}
}

// @Author Alex Lyrakis
bool setModeAttempt(struct pcState *pcState){
	return (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed
		|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed);
}

bool setControlAttempt(struct pcState *pcState){
	return (pcState->escPressed || pcState->aPressed || pcState->zPressed || pcState->qPressed || pcState->wPressed || pcState->upPressed || pcState->downPressed ||
		pcState->leftPressed || pcState->rightPressed || pcState->qPressed || pcState->wPressed);
}

bool setPAttempt(struct pcState *pcState){
	return (pcState->uPressed || pcState->jPressed || pcState->iPressed || pcState->kkPressed || pcState->oPressed || pcState->lPressed);
}

bool sthPressed(struct pcState *pcState){
	return (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed
				|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed || pcState->aPressed || pcState->zPressed || pcState->qPressed ||
				 pcState->wPressed || pcState->uPressed || pcState->jPressed || pcState->iPressed || pcState->kkPressed || pcState->oPressed ||
				 pcState->lPressed || pcState->leftPressed || pcState->rightPressed || pcState->upPressed || pcState->downPressed || pcState->escPressed ||
				 pcState->jThrottleUp || pcState->jThrottleDown || pcState->jLeft || pcState->jRight || pcState->jForward || pcState->jBackward ||
				 pcState->jTwistClockwise || pcState->jTwistCounterClockwise || pcState->jFire);
}

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
			sPacket->payload[2] = pcState->tRollValue;
			sPacket->payload[3] = pcState->tPitchValue;
			sPacket->payload[4] = pcState->tYawValue;
			sPacket->payload[5] =(uint8_t) (pcState->tLiftValue >> 8);
			sPacket->payload[6] =(uint8_t) (pcState->tLiftValue & 0xFF);
			for (int i = 7; i < PACKET_BODY_LENGTH; i++){
				sPacket->payload[i] = 0; // null bytes
			}
			break;
		case 9:
			sPacket->payload[2] = (pcState->P1Value >> 8);
			sPacket->payload[3] = (pcState->P1Value & 0xFF);
			sPacket->payload[4] = (pcState->P2Value >> 8);
			sPacket->payload[5] = (pcState->P2Value & 0xFF);
			sPacket->payload[6] = (pcState->PValue >> 8);
			sPacket->payload[7] = (pcState->PValue & 0xFF);
			for (int i = 8; i < PACKET_BODY_LENGTH; i++){
				sPacket->payload[i] = 0; // null bytes
			}
	}
	// Set crc
	sPacket->crc = 0x00;
	sPacket->crc = crc8_table[sPacket->crc ^ ((uint8_t) (sPacket->fcs >> 8))];
	sPacket->crc = crc8_table[sPacket->crc ^ ((uint8_t) (sPacket->fcs & 0xFF))];
	for (int i=0; i<10; i++) {
		sPacket->crc = crc8_table[sPacket->crc ^ sPacket->payload[i]];
	}
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
	rs232_putchar(sPacket.crc);
}

//@Author George Giannakaras
void receivePacket(SRPacket *rPacket){
	uint8_t crc = 0x00, crcTemp;
	bool foundPacket = false;

	while(pcReQueue.count >= PACKET_LENGTH && foundPacket == false){
		if(queuePeekpc(&pcReQueue, 0) == PREAMPLE_B1 && queuePeekpc(&pcReQueue, 1) == PREAMPLE_B2){
			for(uint16_t j = 2; j < PACKET_LENGTH - 1; j++){
				crc = crc8_table[crc ^ queuePeekpc(&pcReQueue, j)];
			}
			crcTemp = queuePeekpc(&pcReQueue, PACKET_LENGTH - 1);
			if(crc == crcTemp){
				foundPacket = true;
				//discard preamble bytes
				dequeuepc(&pcReQueue);
				dequeuepc(&pcReQueue);

				rPacket->fcs = dequeuepc(&pcReQueue) << 8;
				rPacket->fcs = rPacket->fcs | dequeuepc(&pcReQueue);
				for(int j = 0; j < PACKET_BODY_LENGTH; j++){
					rPacket->payload[j] = dequeuepc(&pcReQueue);
				}
				//discard crc
				dequeuepc(&pcReQueue);
				switch(rPacket->payload[0]) {

					case 2:
						logReceivePacket(rPacket);
						break;
					case 7:
						logReceivePacket(rPacket);
						break;
					case 10:
						logReceivePacket(rPacket);
						break;
					case 11:
						//Ack
						receivedACK[rPacket->fcs] = true;
						break;

				}
			}
			else{
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
void logReceivePacket(SRPacket *rPacket){
	uint16_t motor[4];

	switch(rPacket->payload[0]){
		case 2:
			printf("System time: %hhu | Packet number: %hu | Type: %hhu | Mode: %hhu | Battery: %hhu | Roll: %hhu | Pitch: %hhu | Height: %hhu\n",
				rPacket->payload[6], rPacket->fcs, rPacket->payload[0], rPacket->payload[1], rPacket->payload[2], rPacket->payload[3], rPacket->payload[4], rPacket->payload[5]);
			fprintf(Rfile, "System time: %hu | Packet number: %hu | Type: %hhu | Mode: %hu | Battery: %hu | Roll: %hu | Pitch: %hu | Height: %hu\n",
				rPacket->payload[6], rPacket->fcs, rPacket->payload[0], rPacket->payload[1], rPacket->payload[2], rPacket->payload[3], rPacket->payload[4], rPacket->payload[5]);
			break;
		case 7:
			printf("Type: %hhu | ERROR: %hhu\n", rPacket->payload[0], rPacket->payload[1]);
			fprintf(Rfile, "Type: %hhu | ERROR: %hu\n", rPacket->payload[0], rPacket->payload[1]);
			break;
		case 10:
			motor[0] = rPacket->payload[1] << 8 | rPacket->payload[2];
			motor[1] = rPacket->payload[3] << 8 | rPacket->payload[4];
			motor[2] = rPacket->payload[5] << 8 | rPacket->payload[6];
			motor[3] = rPacket->payload[7] << 8 | rPacket->payload[8];
			printf("Packet number: %hu | Type: %hhu | Motor1: %hu | Motor2: %hu | Motor3: %hu | Motor4: %hu\n",
				rPacket->fcs, rPacket->payload[0], motor[0], motor[1], motor[2], motor[3]);
			fprintf(Rfile, "Packet number: %hu | Type: %hhu | Motor1: %hu | Motor2: %hu | Motor3: %hu | Motor4: %hu\n",
				rPacket->fcs, rPacket->payload[0], motor[0], motor[1], motor[2], motor[3]);
			break;
	}
}

//@Author Alex Lyrakis
void logSendPacket(SRPacket sPacket){
	fprintf(Sfile, "Packet number: %hu ", sPacket.fcs);
	fprintf(Sfile, "Type: %hhu ", sPacket.payload[0]);
	for (int i=1; i<10; i++)
		fprintf(Sfile, "Byte%d: %hhu ", i, sPacket.payload[i]);
	fprintf(Sfile, "crc: %hhu \n", sPacket.crc);
}

void updatePcState(struct pcState *pcState){
	pcState->tLiftValue = pcState->liftValue + pcState->jThrottleValue;
	pcState->tRollValue = pcState->rollValue + pcState->jRollValue - 90;
	pcState->tPitchValue = pcState->pitchValue + pcState->jPitchValue - 90;
	pcState->tYawValue = pcState->yawValue + pcState->jYawValue - 90;
	if (pcState->tLiftValue > 1000)
		pcState->tLiftValue = 1000;
	if (pcState->tRollValue > 180)
		pcState->tRollValue = 180;
	if (pcState->tPitchValue > 180)
		pcState->tPitchValue = 180;
	if (pcState->tYawValue > 180)
		pcState->tYawValue = 180;
}

void initReceivedACK(){
	for(int i = 0; i < 65535; i++){
		receivedACK[i] = false;
	}
}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	//Initialize parameters

	struct pcState *pcState;
	pcState = (struct pcState*) calloc(1, sizeof(struct pcState));
	SRPacket sPacket;
	SRPacket rPacket;
	bool bufferCleared = false;
	char c;
	clock_t timeLastPacket = clock();

	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	term_puts("Initialized termios...\n");
	rs232_open();
	term_puts("Initialized termios...\n");
	openJoystick();
	term_puts("Initialized termios...\n");

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	term_puts("Empty usb buffer\n");
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
	initLogFiles();
	initReceivedACK();
	initPcState(pcState);
	resetPcState(pcState); // Reset values and State of PC side.
	init_queuepc(&pcReQueue);

	for (;;)
	{
		if ((c = term_getchar_nb()) != -1)	// Read from keyboard and store in fd_RS232
		{
			checkInput(c, pcState);
      term_putchar(c);
		}

		// Read from joystic and update pcState
		checkJoystick(pcState);
		updatePcState(pcState);

		// Read from fd_RS232
		if ((c = rs232_getchar_nb()) != -1){
			if(c == 0x13){
				bufferCleared = true;
			}
			if(bufferCleared) {
				enqueuepc(&pcReQueue, (uint8_t) c);
			}
			if(pcReQueue.count >= PACKET_LENGTH) {
				receivePacket(&rPacket);
			}
		//	term_putchar(c);
		}

		if ((clock()-timeLastPacket)> 10)
		{
			//TBD: Based on our pcState and protocol we have to put a sequence of bytes using rs232_putchar(c);
			//		After we have to reset the pcState.
			if (sthPressed(pcState) || pcState->jChanged){
					setPacket(pcState, &sPacket);
					sendPacket(sPacket);
					logSendPacket(sPacket);
					resetPcState(pcState);
					sPacketBuffer[sPacket.fcs] = sPacket;
			}
			timeLastPacket = clock();
		}
	}

	term_exitio();
	rs232_close();
	fclose(Rfile);
	fclose(Sfile);
	term_puts("\n<exit>\n");

	return 0;
}
