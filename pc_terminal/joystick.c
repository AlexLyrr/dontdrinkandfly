#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "pc_terminal.h"
#include "joystick.h"

void getJoystick(struct pcState *pcState)
{
	int fd;
  struct js_event js;
  uint16_t jValue;
  int i = 0;

  if ((fd = open("/dev/input/by-id/usb-Logitech_Logitech_Extreme_3D-joystick", O_RDONLY)) < 0) {
    perror("JoystickError");
    exit(1);
  }
  printf("Joystick Found!\n");

  while(i<10){
    if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
      perror("\nJoystickError: error reading");
      exit (1);
    }
    switch(js.type & ~JS_EVENT_INIT) {
      case JS_EVENT_BUTTON:
        if (js.number == 0){
          pcState->jFire = js.value;
        }
        break;
      case JS_EVENT_AXIS:
        jValue = (js.value+32000)/355;
        switch(js.number) {
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
            pcState->jThrottleValue = (js.value+32000)/64;
          default:
            perror("\nError, non defined axis as input");
            break;
        }
        break;
    }
    i++;
  }
}
