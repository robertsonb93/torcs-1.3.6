#pragma once



#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>
#include <iostream>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>
#include <portability.h>


#include "pit.h"
#include "learn.h"
#include "strategy.h"
#include "cardata.h"
#include "opponent.h"



#define MYBOT_SECT_PRIV "MYBOT private"
#define MYBOT_ATT_FUELPERLAP "fuelperlap"
#define MYBOT_ATT_MUFACTOR "mufactor"
#define MYBOT_ATT_PITTIME "pittime"
#define MYBOT_ATT_BESTLAP "bestlap"
#define MYBOT_ATT_WORSTLAP "worstlap"
#define MYBOT_ATT_TEAMMATE "teammate"


//We have some circular logic in the use here
class Opponent;
class Opponents;
class AbstractStrategy;
class Pit;



class Driver
{
public:
	Driver(int index);
	~Driver();

	//Callback functions called from TORCS
	void initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s);
	void newRace(tCarElt* car, tSituation *s);
	void drive(tSituation *s);
	int pitCommand(tSituation *s);
	void endRace(tSituation *s);

	tCarElt *getCarPtr() { return car; }
	tTrack *getTrackPtr() { return track; }
	float getSpeed() { return mycardata->getSpeedInTrackDirection(); }

private:

	//For Opening and Handling the TheCherries DLL
	static void DoDatDLL();

	//Utility Functions.
	double* setActions();
	double* setState();
	void updateState(tSituation *s);


	bool isStuck();
	void getUnstuck();

	float getAllowedSpeed(tTrackSeg *segment);
		float getAccel();
	float getDistToSegeEnd();
	float getBrake();
	int getGear();
	float getSteer();
	float getClutch();
	vec2f getTargetPoint();
	float getOffset();
	float brakedist(float allowedspeed, float mu);

	//Filter Utility Funcs.
	float filterOverlap(float accel);
	float filterBColl(float brake);
	float filterABS(float brake);
	float filterBPit(float brake);
	float filterBrakeSpeed(float brake);
	float filterTurnSpeed(float brake);

	float filterTCL(float accel);
	float filterTrk(float accel);
	float filterSColl(float steer);

	float filterTCL_RWD();
	float filterTCL_FWD();
	float filterTCL_4WD();
	void initTCLfilter();

	void initCa();
	void initCw();
	void initTireMu();

	void computeRadius(float *radius);
	int isAlone();

	int stuck;
	float speedangle;		// the angle of the speed vector relative to trackangle, > 0.0 points to right.
	float mass;				// Mass of car + fuel.
	float myoffset;			// Offset to the track middle.
	tCarElt *car;		// Pointer to tCarElt struct.

	Opponents *opponents;	// The container for opponents.
	Opponent* opponent; 	// The array of opponents.

	Pit *pit;						// Pointer to the pit instance.
	AbstractStrategy *strategy;		// Pit stop strategy.

	static Cardata *cardata;		// Data about all cars shared by all instances.
	SingleCardata *mycardata;		// Pointer to "global" data about my car.
	static double currentsimtime;	// Store time to avoid useless updates.

	float currentspeedsqr;	// Square of the current speed_x.
	float clutchtime;		// Clutch timer.
	float oldlookahead;		// Lookahead for steering in the previous step.

	float *radius;
	SegLearn *learn;
	int alone;

	// Data that should stay constant after first initialization.
	int MAX_UNSTUCK_COUNT;
	int INDEX;
	float CARMASS;		// Mass of the car only [kg].
	float CA;			// Aerodynamic downforce coefficient.
	float CW;			// Aerodynamic drag coefficient.
	float TIREMU;		// Friction coefficient of tires.
	float (Driver::*GET_DRIVEN_WHEEL_SPEED)();
	float OVERTAKE_OFFSET_INC;		// [m/timestep]
	float MU_FACTOR;				// [-]

									// Class constants.
	static const float MAX_UNSTUCK_ANGLE;
	static const float UNSTUCK_TIME_LIMIT;
	static const float MAX_UNSTUCK_SPEED;
	static const float MIN_UNSTUCK_DIST;
	static const float G;
	static const float FULL_ACCEL_MARGIN;
	static const float SHIFT;
	static const float SHIFT_MARGIN;
	static const float ABS_SLIP;
	static const float ABS_RANGE;
	static const float ABS_MINSPEED;
	static const float TCL_SLIP;
	static const float LOOKAHEAD_CONST;
	static const float LOOKAHEAD_FACTOR;
	static const float WIDTHDIV;
	static const float SIDECOLL_MARGIN;
	static const float BORDER_OVERTAKE_MARGIN;
	static const float OVERTAKE_OFFSET_SPEED;
	static const float PIT_LOOKAHEAD;
	static const float PIT_BRAKE_AHEAD;
	static const float PIT_MU;
	static const float MAX_SPEED;
	static const float TCL_RANGE;
	static const float MAX_FUEL_PER_METER;
	static const float CLUTCH_SPEED;
	static const float CENTERDIV;
	static const float DISTCUTOFF;
	static const float MAX_INC_FACTOR;
	static const float CATCH_FACTOR;
	static const float CLUTCH_FULL_MAX_TIME;
	static const float USE_LEARNED_OFFSET_RANGE;

	static const float TEAM_REAR_DIST;
	static const int TEAM_DAMAGE_CHANGE_LEAD;

	// Track variables.
	tTrack* track;



};

#endif
