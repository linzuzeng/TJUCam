#pragma once
/*
*
* SIXENSE CONFIDENTIAL
*
* Copyright (C) 2011 Sixense Entertainment Inc.
* All Rights Reserved
*
*/
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>

#ifndef _SIXENSE_H_
#define _SIXENSE_H_

#define SIXENSE_BUTTON_BUMPER   (0x01<<7)
#define SIXENSE_BUTTON_JOYSTICK (0x01<<8)
#define SIXENSE_BUTTON_1        (0x01<<5)
#define SIXENSE_BUTTON_2        (0x01<<6)
#define SIXENSE_BUTTON_3        (0x01<<3)
#define SIXENSE_BUTTON_4        (0x01<<4)
#define SIXENSE_BUTTON_START    (0x01<<0)

#define SIXENSE_SUCCESS 0
#define SIXENSE_FAILURE -1

#define SIXENSE_MAX_CONTROLLERS 4

typedef struct _sixenseControllerData {
	float pos[3];
	float joystick_x;
	float joystick_y;
	float trigger;
	unsigned int buttons1;
	unsigned int buttons2;
	unsigned int buttons3;
	unsigned int buttons4;
	unsigned int buttons5;
	unsigned int buttons6;
	unsigned int buttons7;
	unsigned char sequence_number;
	float rot_quat[4];
	unsigned short firmware_revision;
	unsigned short hardware_revision;
	unsigned short packet_type;
	unsigned short magnetic_frequency;
	int enabled;
	int controller_index;
	unsigned char is_docked;
	unsigned char which_hand;
	unsigned char hemi_tracking_enabled;
} sixenseControllerData;

typedef struct _sixenseAllControllerData {
	sixenseControllerData controllers[4];
} sixenseAllControllerData;


	 int lzzvrInit(void);
	 int lzzvrExit(void);

	 int sixenseGetMaxBases();
	 int sixenseSetActiveBase(int i);
	 int sixenseIsBaseConnected(int i);

	 int sixenseGetMaxControllers(void);
	 int sixenseIsControllerEnabled(int which);
	
	 //int sixenseGetNewestData(int which, sixenseControllerData *);
	 int sixenseGetAllNewestData(sixenseAllControllerData *);

	 int sixenseAutoEnableHemisphereTracking(int which_controller);

	 int sixenseSetHighPriorityBindingEnabled(int on_or_off);
	
	 int sixenseTriggerVibration(int controller_id, int duration_100ms, int pattern_id);

#endif /* _SIXENSE_H_ */