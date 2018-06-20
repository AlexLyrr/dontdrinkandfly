#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <termios.h>

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <linux/serial.h>
#include <sys/ioctl.h>

#include "pc_terminal.h"

/**
 *	@author Joseph Verburg 
 */
void rs232_open(void) {
	char* name;
	int result;
	struct termios tty;

	fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY); // Hardcode your serial port here, or request it as an argument at runtime
	assert(fd_RS232 >= 0);

	fcntl(fd_RS232, F_SETFL, FNDELAY);

	result = isatty(fd_RS232);
	assert(result == 1);

	name = ttyname(fd_RS232);
	assert(name != 0);

	result = tcgetattr(fd_RS232, &tty);
	assert(result == 0);

	tty.c_iflag = IGNBRK; /* ignore break condition */
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	tty.c_oflag = 0;

	tty.c_lflag = 0;
	// tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
	tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */
	tty.c_cflag &= ~CRTSCTS;

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	struct serial_struct serial;
	ioctl(fd_RS232, TIOCGSERIAL, &serial);
	serial.flags |= ASYNC_LOW_LATENCY;
	ioctl(fd_RS232, TIOCSSERIAL, &serial);

	result = tcsetattr(fd_RS232, TCSANOW, &tty); /* non-canonical */

	tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}

void rs232_close(void) {
	int result;

	result = close(fd_RS232);
	assert(result == 0);
}

int rs232_getchar_nb() {
	int result;
	unsigned char c;

	result = read(fd_RS232, &c, 1);

	if (result == 0)
		return -1;

	else {
		assert(result == 1);
		return (int)c;
	}
}

int rs232_getchar() {
	int c;

	while ((c = rs232_getchar_nb()) == -1)
		;
	return c;
}

int rs232_putchar(char c) {
	int result;

	do {
		result = (int)write(fd_RS232, &c, 1);
	} while (result == 0);

	assert(result == 1);
	return result;
}
