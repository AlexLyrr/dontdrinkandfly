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
#include <stdbool.h> 


/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
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
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include "pc_terminal.h"
#include <time.h>
#include <time.h>
#include <stdlib.h>


int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
  	char 		*name;
  	int 		result;
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
}


// @Author: George Giannakaras 
void init_pcState(struct pcState *pcState){ 
  pcState->liftValue = 0; 
  pcState->rollValue = 90; 
  pcState->pitchValue = 90; 
  pcState->yawValue = 90; 
} 

// @Author: Alex Lyrakis
void check_input(char c, struct pcState *pcState)
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
						pcState->upPressed = true;
						if (pcState->pitchValue > 0)
							pcState->pitchValue -= 1;
						break;
					case 'B':
						pcState->escPressed = false;
						pcState->downPressed = true;
						if (pcState->pitchValue < 180)
							pcState->pitchValue += 1;
						break;
					case 'C':
						pcState->escPressed = false;
						pcState->rightPressed = true;
						if (pcState->rollValue > 0)
							pcState->rollValue -= 1;
						break;
					case 'D':
						pcState->escPressed = false;
						pcState->leftPressed = true;
						if (pcState->rollValue < 180)
							pcState->rollValue += 1;
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
			pcState->aPressed = true;
			if (pcState->liftValue <=1000)
				pcState->liftValue +=10;
			break;
		case 'z':
			pcState->zPressed = true;
			if (pcState->liftValue <=1000)
				pcState->liftValue -=10;
			break;
		case 'q':
			pcState->qPressed = true;
			break;
		case 'w':
			pcState->wPressed = true;
			break;
		case 'u':
			pcState->uPressed = true;
			break;
		case 'j':
			pcState->jPressed = true;
			break;
		case 'i':
			pcState->iPressed = true;
			break;
		case 'k':
			pcState->kkPressed = true;
			break;
		case 'o':
			pcState->oPressed = true;
			break;
		case 'l':
			pcState->lPressed = true;
			break;
	}
}


void sendPacket(struct pcState *pcState){
	//Define packet type
	uint8_t frameType = 0;
	if (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed 
		|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed)
		frameType = 5;
	if (pcState->aPressed || pcState->zPressed || pcState->qPressed || pcState->wPressed || pcState->upPressed || pcState->downPressed || 
		pcState->leftPressed || pcState->rightPressed)
		frameType = 3;
	rs232_putchar(frameType); //Send frameType Byte

	switch (frameType) {
		case 5:
			//here we must send the packet
			rs232_putchar(pcState->mode);
			for (int i = 0; i < 8; i++) {
				rs232_putchar(0);
			}
			break;
		case 3:
			if (pcState->escPressed) {
				rs232_putchar(0x80);	// abort byte
			} else {
				rs232_putchar(0);  // else zero
			}
			rs232_putchar(pcState->rollValue); // roll byte
			rs232_putchar(pcState->pitchValue);
			rs232_putchar(pcState->yawValue);
			rs232_putchar(pcState->liftValue >> 8);
			rs232_putchar(pcState->liftValue & 0xFF);
			for (int i=0; i<3; i++) {
				rs232_putchar(0); // send 3 remaining null bytes
			}
			break;

	}	

}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char c;
	clock_t timeLastPacket=0; //= clock();

	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	term_puts("Initialized termios...\n");
	rs232_open();
	term_puts("Initialized termios...\n");

	term_puts("Type ^C to exit\n");
	struct pcState *pcState;
	pcState = (struct pcState*) calloc(1, sizeof(struct pcState));
	
	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
	init_pcState(pcState);
	resetPcState(pcState); // Reset values and State of PC side.

	for (;;)
	{
		if ((c = term_getchar_nb()) != -1)	// Read from keyboard and store in fd_RS232
		{
			check_input(c, pcState);
		}
		if ((c = rs232_getchar_nb()) != -1)	// Read from fd_RS232 and 
			//term_putchar(c);
		if ((clock()-timeLastPacket)> 50)
		{
			//TBD: Based on our pcState and protocol we have to put a sequence of bytes using rs232_putchar(c);
			//		After we have to reset the pcState.
			if (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed 
		|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed || pcState->aPressed || pcState->zPressed || pcState->qPressed ||
		 pcState->wPressed || pcState->uPressed || pcState->jPressed || pcState->iPressed || pcState->kkPressed || pcState->oPressed || 
		 pcState->lPressed){
				sendPacket(pcState);
				resetPcState(pcState);
			}
			timeLastPacket = clock();
		}
	}

	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

