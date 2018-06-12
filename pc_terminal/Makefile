
CC=gcc
CFLAGS = -g -Wall -lm 
EXEC = ./pc-terminal

all:
	$(CC) $(CFLAGS) -D GUIACTIVATED pc_terminal.c pcqueue.c keyboard.c rs232.c eslgui.c -o $(EXEC) `pkg-config --cflags --libs gtk+-3.0 gthread-2.0 gmodule-2.0` -pthread
	#$(CC) $(CFLAGS) pc_terminal.c pcqueue.c joystick.c -lm eslgui.c -o $(EXEC) `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -pthread

joystick:
	$(CC) $(CFLAGS) -D GUIACTIVATED -D JOYSTICK_ENABLE pc_terminal.c pcqueue.c keyboard.c rs232.c joystick.c -lm eslgui.c -o $(EXEC) `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -pthread

ble:
	hciconfig hci0 reset
	$(CC) $(CFLAGS) -D GUIACTIVATED -D BLE_ENABLE pc_terminal.c pcqueue.c keyboard.c rs232.c eslgui.c -o $(EXEC) `pkg-config --cflags --libs gtk+-3.0 gthread-2.0 gmodule-2.0 gattlib` -pthread
	
ble-joystick:
	hciconfig hci0 reset
	$(CC) $(CFLAGS) -D GUIACTIVATED -D BLE_ENABLE -D JOYSTICK_ENABLE pc_terminal.c pcqueue.c keyboard.c rs232.c joystick.c -lm eslgui.c -o $(EXEC) `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0 gattlib` -pthread


computer: all

run:
	$(EXEC)


jstest:
	$(CC) $(CFLAGS) jstest.c -o ./jstest
	./jstest --normal /dev/input/by-id/usb-Logitech_Logitech_Extreme_3D-joystick

