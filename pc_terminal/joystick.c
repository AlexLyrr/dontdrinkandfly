#include "joystick.h"
#include "pc_terminal.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//void getJoystick(struct pcState *pcState)
void openJoystick() {
	/* Initializations moves to pc_terminal.c
	int fd;
  int i = 0;
  */
	if ((fd_joystick = open("/dev/input/by-id/usb-Logitech_Logitech_Extreme_3D-joystick", O_RDONLY)) < 0) {
		perror("JoystickError");
		exit(1);
	}
	printf("Joystick Found!\n");
	fcntl(fd_joystick, F_SETFL, O_NONBLOCK);
}

void checkJoystick(struct pcState *pcState) {
	struct js_event js;
	uint16_t jValue;
	int32_t jTemp;
	//while(read(fd, &js, sizeof(struct js_event)) == sizeof(struct js_event)) {
	if (read(fd_joystick, &js, sizeof(struct js_event)) == sizeof(struct js_event)) {
		pcState->jChanged = true;
		jTemp = js.value;
		if (jTemp > 32000) {
			jTemp = 32000;
		} else if (jTemp < -32000) {
			jTemp = -32000;
		}
		switch (js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				if (js.number == 0) {
					pcState->jFire = js.value == 1;
				}
				break;
			case JS_EVENT_AXIS:
				jTemp += 32000;
				jValue = jTemp / 355;
				switch (js.number) {
					case 0:
						pcState->jRollValue = 180 - jValue;
						break;
					case 1:
						pcState->jPitchValue = jValue;
						break;
					case 2:
						pcState->jYawValue = jValue;
						break;
					case 3:
						if (jTemp > 60800) {
							pcState->jThrottleValue = 0;
						} else {
							// jTemp = (sqrt((double)(60800 - jTemp)) * 4) / 2;
							jTemp = (sqrt((double)(60800 - jTemp)) * 6) / 3;
							// if (jTemp > 32000) {
							//   jTemp = (60800-jTemp)/160;
							// } else{
							//   jTemp = (32000-x)/120 + 200;
							// }
							pcState->jThrottleValue = jTemp + 200;
						}
						break;
					default:
						perror("\nError, non defined axis as input");
						break;
				}
				break;
		}
	}
}
