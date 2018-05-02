// @Author: George Giannakaras

#ifndef PC_TERMINAL_H_  
#define PC_TERMINAL_H_

struct pcState{
	//KEYBOARD
	bool escPressed; //abort / exit
	//modes
	bool n0Pressed;
	bool n1Pressed;
	bool n2Pressed;
	bool n3Pressed;
	bool n4Pressed;
	bool n5Pressed;
	bool n6Pressed;
	bool n7Pressed;
	bool n8Pressed;

	bool aPressed;	//lift up
	bool zPressed;	//lift down
	uint16_t liftValue;

	bool leftPressed;	//roll up
	bool rightPressed;	//roll down
	uint8_t rollValue;
	bool upPressed;		//pitch down
	bool downPressed;	//pitch up
	uint8_t pitchValue;

	bool qPressed;	//yaw down
	bool wPressed;	//yaw up
	uint8_t yawValue;

	bool uPressed;	//yaw control P up
	bool jPressed;	//yaw control P down

	bool iPressed;	//roll/pitch control P1 up
	bool kkPressed;	//roll/pitch control P1 down

	bool oPressed;	//roll/pitch control P2 up
	bool lPressed;	//roll/pitch control P2 down

	//JOYSTICK
	bool jThrottleUp;	//lift up
	bool jThrottleDown;	//lift down
	bool jLeft;		//roll up
	bool jRight;	//roll down
	bool jForward;	//pitch down
	bool jBackward;	//pitch up
	bool jTwistClockwise;			//yaw up
	bool jTwistCounterClockwise;	//yaw down
	bool jFire;	//abort / exit
};

#endif 