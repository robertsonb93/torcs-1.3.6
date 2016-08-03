#include "cardata.h"
#if _MSC_VER < 1900
#define nullptr NULL
#endif

//Update will update all the protected members of the singlecardata class
void SingleCardata::update()
{
	trackangle = RtTrackSideTgAngleL((&car->_trkPos));
	speed = getSpeed(car, trackangle);
	angle = trackangle - car->_yaw;
	NORM_PI_PI(angle);
	width = (car->_dimension_x*sin(angle)) + (car->_dimension_y*cos(angle));

}

//Just do some trig to find what the current velocity is for the car, then find out how the velocity relates to the track, this is the speed along the track.
float SingleCardata::getSpeed(tCarElt * car, float ltrackangle)
{
	vec2f speed, dir;
	speed.x = car->_speed_X;
	speed.y = car->_speed_Y;
	dir.x = cos(ltrackangle);
	dir.y = sin(ltrackangle);
	return speed*dir;
}


//What does a tSituation contain? My guess is it has all the cars, perhaps track data as well.
//So we are grabbing the number of cars, then creating an array of SingleCardata, a slot for each of the race cars
//Then we loops through the cars and initialize our data array with them. Each containing the trackangle,speed etc. as seen above. 
Cardata::Cardata(tSituation *s)
{
	ncars = s->_ncars;
	data = new SingleCardata[ncars];
	int i;
	for (i = 0; i < ncars; i++)
	{
		data[i].init(s->cars[i]);
	}
}

//Just delete the data pointer we had created to prevent memory leaks 
Cardata::~Cardata()
{
	delete[] data;
}

//Call up the update function from SingleCarData, this will update each cars information in the race
void Cardata::update()
{
	int i;
	for (i = 0; i < ncars; i++)
	{
		data[i].update();
	}
}

//We are looking for our car, so we use the car pointer, and pass it to thisCar on every car, till one matches.
//If no cars are a match, then we will return a nullptr, But I assume this means something is wrong when your car is no longer in teh race.
SingleCardata * Cardata::findCar(tCarElt * car)
{
	int i;
	for (i = 0; i < ncars; i++) {
		if (data[i].thisCar(car)) {
			return &data[i];
		}
	}
	return nullptr;
}
