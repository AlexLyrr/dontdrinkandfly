
CC=gcc
CFLAGS = -g -Wall -lm
EXEC = ./pc-terminal

all:
	$(CC) $(CFLAGS) pc_terminal.c pcqueue.c -o $(EXEC)

run: all
	$(EXEC)


jstest:
	$(CC) $(CFLAGS) jstest.c -o ./jstest
	./jstest --normal /dev/input/by-id/usb-Logitech_Logitech_Extreme_3D-joystick

joystick:
	$(CC) $(CFLAGS) joystick.c -o ./joystick
	./joystick
