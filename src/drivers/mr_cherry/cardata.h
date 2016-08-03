#pragma once



#ifndef _MR_CHERRY_CARDATA_H
#define _MR_CHERRY_CARDATA_H

/*
This class holds global facts about cars, therefore no data relative to
each other (for that is the class Opponents/Opponent responsible).
*/
//
#include <stdio.h>
#include <math.h>
#include <car.h>
#include <robottools.h>
#include <raceman.h>




class  SingleCardata
{
public:
	inline void init(CarElt *car) { this->car = car; };
	inline float getSpeedInTrackDirection() { return speed; }
	inline float getWidthOnTrack() { return width; }
	inline float getTrackangle() { return trackangle;  }
	inline float getCarAngle() { return angle; }
	inline bool thisCar(tCarElt *car) { return (car == this->car); }

	void update();

protected:
	static float getSpeed(tCarElt *car, float trackangle);

	float speed; //Velocity in regards to direction of the track
	float width; //The car has a needed width on the tracked
	float trackangle; //Track angle at the opponents position, I am not exactly what the opponent is
	float angle; //the angle of the car relative to the track tangent.

	tCarElt *car; //A Pointer to the car from elsewhere



};

class  Cardata {
public:
	Cardata(tSituation *s);
	~Cardata();

	void update();
	SingleCardata *findCar(tCarElt *car);

protected:
	SingleCardata *data;	// Array with car data.
	int ncars;				// # of elements in data.
};


#endif