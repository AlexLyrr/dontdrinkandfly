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
#include "pc_terminal.h"

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
void check_input(char c, struct *pcState)
{
	switch (c)
	{
		case 27:
			pcState->escPressed = true;
			c = term_getchar_nb();
			if (c == 91){					// If we have one 'Esc' and '[' in a raw, that means it is probably an arrow command.
				c = term_getchar_nb();
				switch (c)
				{
					case 'A':
						pcState->escPressed = false;
						pcState->upPressed = true;
						break;
					case 'B':
						pcState->escPressed = false;
						pcState->downPressed = true;
						break;
					case 'C':
						pcState->escPressed = false;
						pcState->rightPressed = true;
						break;
					case 'D':
						pcState->escPressed = false;
						pcState->leftPressed = true;
						break;	
				}
			}
			break;
		case '0':
			pcState->n0Pressed = true;
			break;
		case '1':
			pcState->n1Pressed = true;
			break;
		case '2':
			pcState->n2Pressed = true;
			break;
		case '3':
			pcState->n3Pressed = true;
			break;
		case '4':
			pcState->n4Pressed = true;
			break;
		case '5':
			pcState->n5Pressed = true;
			break;
		case '6':
			pcState->n6Pressed = true;
			break;
		case '7':
			pcState->n7Pressed = true;
			break;
		case '8':
			pcState->n8Pressed = true;
			break;
		case '9':
			pcState->n9Pressed = true;
			break;
		case 'a':
			pcState->aPressed = true;
			break;
		case 'z':
			pcState->zPressed = true;
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
		default:
			nrf_gpio_pin_toggle(RED);
	}
}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char	c;
	clock_t timeLastPacket=0; //= clock();

	term_puts("\nTerminal program - Embedded Real-TFime Systems\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
	for (;;)
	{
		if ((c = term_getchar_nb()) != -1)	// Read from keyboard and store in fd_RS232
		{
			check_input(c, &pcState);	
		}
		if ((c = rs232_getchar_nb()) != -1)	// Read from fd_RS232 and 
			term_putchar(c);
		if ((clock()-timeLastPacket)> 50)
		{
			//TBD: Based on our pcState and protocol we have to put a sequence of bytes using rs232_putchar(c);
			//		After we have to reset the pcState.
			timeLastPacket = clock();
		}
	}

	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

