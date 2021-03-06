#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pc_terminal.h"

/**
 *	@author Alex Lyrakis 
 */
void resetPcState(struct pcState* pcState) {
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
	pcState->n9Pressed = false;

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
	pcState->yPressed = false;
	pcState->hPressed = false;
	pcState->tPressed = false;
	pcState->gPressed = false;

	//JOYSTIC
	pcState->jLift = false;
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

/**
 *	@author Georgios Giannakaras 
 */
void initPcState(struct pcState* pcState) {
	pcState->liftValue = 0;
	pcState->rollValue = 90;
	pcState->pitchValue = 90;
	pcState->yawValue = 90;
	pcState->PValue = 80;
	pcState->P1Value = 25;
	pcState->P2Value = 25; //
	pcState->PheightValue = 0;
	pcState->PheightValue2 = 0;

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

/**
 *	@author Alex Lyrakis 
 */
void checkInput(char c, struct pcState* pcState) {
	switch (c) {
		case 27:
			pcState->escPressed = true;
			c = term_getchar_nb();
			if (c == 91) { // If we have one 'Esc' and '[' in a row, that means it is probably an arrow command.
				c = term_getchar_nb();
				switch (c) {
					case 'A':
						pcState->escPressed = false;
						if (pcState->pitchValue > 0 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
							pcState->upPressed = true;
							pcState->pitchValue -= 1;
						}

						break;
					case 'B':
						pcState->escPressed = false;
						if (pcState->pitchValue < 180 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
							pcState->downPressed = true;
							pcState->pitchValue += 1;
						}

						break;
					case 'C':
						pcState->escPressed = false;
						if (pcState->rollValue > 0 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
							pcState->rightPressed = true;
							pcState->rollValue -= 1;
						}

						break;
					case 'D':
						pcState->escPressed = false;
						if (pcState->rollValue < 180 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
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
		case '9':
			pcState->n9Pressed = true;
			pcState->mode = 9;
			break;
		case 'a':
			if (pcState->liftValue < 1000 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->liftValue += 10;
				pcState->aPressed = true;
			}
			break;
		case 'z':
			if (pcState->liftValue > 0 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->zPressed = true;
				pcState->liftValue -= 10;
			}
			break;
		case 'q':
			if (pcState->yawValue >= 10 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->qPressed = true;
				pcState->yawValue -= 1;
			}
			break;
		case 'w':
			if (pcState->yawValue < 180 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->wPressed = true;
				pcState->yawValue += 1;
			}
			break;
		case 'u':
			if (pcState->PValue < 1000 && (pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->uPressed = true;
				pcState->PValue += 1;
			}

			break;
		case 'j':
			if (pcState->PValue > 0 && (pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8)) {
				pcState->jPressed = true;
				pcState->PValue -= 1;
			}
			break;
		case 'i':
			if (pcState->P1Value < 1000 && (pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
				pcState->iPressed = true;
				pcState->P1Value += 1;
			}
			break;
		case 'k':
			if (pcState->P1Value > 0 && (pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
				pcState->kkPressed = true;
				pcState->P1Value -= 1;
			}
			break;
		case 'o':
			if (pcState->P2Value < 1000 && (pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
				pcState->oPressed = true;
				pcState->P2Value += 1;
			}
			break;
		case 'l':
			if (pcState->P2Value > 0 && (pcState->mode == 5 || pcState->mode == 6 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 9)) {
				pcState->lPressed = true;
				pcState->P2Value -= 1;
			}
			break;
		case 'y':
			if (pcState->PheightValue < 1000 && (pcState->mode == 5 || pcState->mode == 7)) {
				pcState->yPressed = true;
				pcState->PheightValue += 1;
			}
			break;
		case 'h':
			if (pcState->PheightValue > 0 && (pcState->mode == 5 || pcState->mode == 7)) {
				pcState->hPressed = true;
				pcState->PheightValue -= 1;
			}
			break;
		case 't':
			if (pcState->PheightValue2 < 256 && (pcState->mode == 5 || pcState->mode == 7)) {
				pcState->tPressed = true;
				pcState->PheightValue2 += 1;
			}
			break;
		case 'g':
			if (pcState->PheightValue2 > 0 && (pcState->mode == 5 || pcState->mode == 7)) {
				pcState->gPressed = true;
				pcState->PheightValue2 -= 1;
			}
			break;
	}
}

/**
 *	@author Alex Lyrakis 
 */
bool setModeAttempt(struct pcState* pcState) {
	return (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed
		|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed || pcState->n9Pressed);
}

bool setControlAttempt(struct pcState* pcState) {
	return (pcState->escPressed || pcState->aPressed || pcState->zPressed || pcState->qPressed || pcState->wPressed || pcState->upPressed || pcState->downPressed || pcState->leftPressed || pcState->rightPressed || pcState->qPressed || pcState->wPressed);
}

bool setPAttempt(struct pcState* pcState) {
	return (pcState->uPressed || pcState->jPressed || pcState->iPressed || pcState->kkPressed || pcState->oPressed || pcState->lPressed || pcState->yPressed || pcState->hPressed || pcState->tPressed || pcState->gPressed);
}

bool sthPressed(struct pcState* pcState) {
	return (pcState->n0Pressed || pcState->n1Pressed || pcState->n2Pressed || pcState->n3Pressed || pcState->n4Pressed || pcState->n5Pressed
		|| pcState->n6Pressed || pcState->n7Pressed || pcState->n8Pressed || pcState->n9Pressed || pcState->aPressed || pcState->zPressed || pcState->qPressed || pcState->wPressed || pcState->uPressed || pcState->jPressed || pcState->iPressed || pcState->kkPressed || pcState->oPressed || pcState->yPressed || pcState->hPressed || pcState->tPressed || pcState->gPressed || pcState->lPressed || pcState->leftPressed || pcState->rightPressed || pcState->upPressed || pcState->downPressed || pcState->escPressed || pcState->jThrottleUp || pcState->jThrottleDown || pcState->jLeft || pcState->jRight || pcState->jForward || pcState->jBackward || pcState->jTwistClockwise || pcState->jTwistCounterClockwise || pcState->jFire);
}
