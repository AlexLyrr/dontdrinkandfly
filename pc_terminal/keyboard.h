#ifndef _LINUX_KEYBOARD_H
#define _LINUX_KEYBOARD_H


void resetPcState(struct pcState *pcState);
void initPcState(struct pcState *pcState);
void checkInput(char c, struct pcState *pcState);
bool setModeAttempt(struct pcState *pcState);
bool setControlAttempt(struct pcState *pcState);
bool setPAttempt(struct pcState *pcState);
bool sthPressed(struct pcState *pcState);

#endif /* _LINUX_KEYBOARD_H */