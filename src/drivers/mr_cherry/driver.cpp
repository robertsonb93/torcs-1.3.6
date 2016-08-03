/*
Based of the bt bot. 
Modified by Brandon Robertson (robertsonb@uleth.ca)
July 11,2016

For someone looking how to control the driver, this is where its at. 
Look under the function "drive" you will see multiple get functions i.e getSpeed
These getters are what determine the various inputs and outputs of the robot.
Good luck.
*/

#include "driver.h"
#if _MSC_VER < 1900
#define nullptr NULL
#endif

const float Driver::MAX_UNSTUCK_ANGLE = 15.0f / 180.0f*PI;	// [radians] If the angle of the car on the track is smaller, we assume we are not stuck.
const float Driver::UNSTUCK_TIME_LIMIT = 1.0f;				// [s] We try to get unstuck after this time.
const float Driver::MAX_UNSTUCK_SPEED = 5.0f;				// [m/s] Below this speed we consider being stuck.
const float Driver::MIN_UNSTUCK_DIST = 3.0f;				// [m] If we are closer to the middle we assume to be not stuck.
const float Driver::G = 9.81f;								// [m/(s*s)] Welcome on Earth.
const float Driver::FULL_ACCEL_MARGIN = 1.0f;				// [m/s] Margin reduce oscillation of brake/acceleration.
const float Driver::SHIFT = 0.9f;							// [-] (% of rpmredline) When do we like to shift gears.
const float Driver::SHIFT_MARGIN = 4.0f;					// [m/s] Avoid oscillating gear changes.
const float Driver::ABS_SLIP = 2.0f;						// [m/s] range [0..10]
const float Driver::ABS_RANGE = 5.0f;						// [m/s] range [0..10]
const float Driver::ABS_MINSPEED = 3.0f;					// [m/s] Below this speed the ABS is disabled (numeric, division by small numbers).
const float Driver::TCL_SLIP = 2.0f;						// [m/s] range [0..10]
const float Driver::TCL_RANGE = 10.0f;						// [m/s] range [0..10]
const float Driver::LOOKAHEAD_CONST = 17.0f;				// [m]
const float Driver::LOOKAHEAD_FACTOR = 0.33f;				// [-]
const float Driver::WIDTHDIV = 2.0f;						// [-] Defines the percentage of the track to use (2/WIDTHDIV).
const float Driver::SIDECOLL_MARGIN = 3.0f;					// [m] Distance between car centers to avoid side collisions.
const float Driver::BORDER_OVERTAKE_MARGIN = 0.5f;			// [m]
const float Driver::OVERTAKE_OFFSET_SPEED = 5.0f;			// [m/s] Offset change speed.
const float Driver::PIT_LOOKAHEAD = 6.0f;					// [m] Lookahead to stop in the pit.
const float Driver::PIT_BRAKE_AHEAD = 200.0f;				// [m] Workaround for "broken" pitentries.
const float Driver::PIT_MU = 0.4f;							// [-] Friction of pit concrete.
const float Driver::MAX_SPEED = 84.0f;						// [m/s] Speed to compute the percentage of brake to apply.
const float Driver::MAX_FUEL_PER_METER = 0.0008f;			// [liter/m] fuel consumtion.
const float Driver::CLUTCH_SPEED = 5.0f;					// [m/s]
const float Driver::CENTERDIV = 0.1f;						// [-] (factor) [0.01..0.6].
const float Driver::DISTCUTOFF = 200.0f;					// [m] How far to look, terminate while loops.
const float Driver::MAX_INC_FACTOR = 5.0f;					// [m] Increment faster if speed is slow [1.0..10.0].
const float Driver::CATCH_FACTOR = 10.0f;					// [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
const float Driver::CLUTCH_FULL_MAX_TIME = 2.0f;			// [s] Time to apply full clutch.
const float Driver::USE_LEARNED_OFFSET_RANGE = 0.2f;		// [m] if offset < this use the learned stuff

const float Driver::TEAM_REAR_DIST = 50.0f;					//
const int Driver::TEAM_DAMAGE_CHANGE_LEAD = 700;			// When to change position in the team?

															// Static variables.
Cardata *Driver::cardata = NULL;

double Driver::currentsimtime;
double trackangle;
double angle;

typedef int(__cdecl *MYPROC)(LPWSTR);
//We have to prototype the Type of functions that will be called from TheCherries DLL.
typedef double* (__cdecl *PerformAction)();
typedef void(__cdecl *PerformUpdate)(const double* StatePrime, const double Reward);
typedef void(__cdecl *cherryEntry)(const double* AvailActions, const int ActSize, const int AAsz, const double* StartState, const int SSsz);
static PerformAction CherryAction;
static cherryEntry EnterTheCherry;
static PerformUpdate CherryUpdate;
static HINSTANCE TheCherriesDLLHandle;

//Members used to pass info to the Cherry,
static double* AvailActions;
static double AAsz, ActSize, stateSize;
double bestLapTime;
int lastLap;
int oldSeg;
double* oldstate, *state;
double*action = new double[2];
bool offTrack = false, midTrack = true;
int numUpdates = 0;
bool firstStep = true;
int delayUpdate = 0;
int updatesWhileStopped = 0;
int unStuckProcedure = 0;
double lastStuckDistance = 0;
int tryProcedure = 0;

bool commitLapTimeWasTrue = true;


Driver::Driver(int index) 
{
	INDEX = index;
	DoDatDLL();
}

Driver::~Driver()
{
	delete opponents;
	delete pit;
	delete[] radius;
	delete learn;
	delete strategy;
	if(cardata != nullptr)
	{
		delete cardata;
		cardata = nullptr;
	}
	FreeLibrary(TheCherriesDLLHandle);
	CherryUpdate = NULL;
	CherryAction = NULL;
	EnterTheCherry = NULL;

}


//called for every track change or new race.
//Initalize the track,
//Then init the race type, 
//Will also 
void Driver::initTrack(tTrack * t, void * carHandle, void ** carParmHandle, tSituation * situation)
{
	//track takes the value of pointer
	track = t;

	const int BUFSIZE = 256; //A Magic number, consider looking into its meaning
	char buffer[BUFSIZE];

	//Load a custom setup if one is available.
	//Get a pointer to the first char of the track filename

	char* trackname = strrchr(track->filename, '/') + 1; // returns a pointer of the last occurance of '/' given the track->filename

	switch (situation->_raceType)
	{
	case RM_TYPE_PRACTICE: snprintf(buffer, BUFSIZE, "drivers/mr_cherry/%d/practice/%s", INDEX, trackname); break;
	case RM_TYPE_QUALIF: snprintf(buffer, BUFSIZE, "drivers/mr_cherry/%d/qualifying/%s", INDEX, trackname); break;
	case RM_TYPE_RACE: snprintf(buffer, BUFSIZE, "driver/mr_cherry/%d/race/%s", INDEX, trackname); break;
	default: break;
	}

	*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);// A function from TGF, i believe it is the parameters of the car

	if (*carParmHandle == nullptr || *carParmHandle == NULL) // Used both to as I assume the GfParmReadFile may return NULL
	{
		snprintf(buffer, BUFSIZE, "drivers/mr_cherry/%d/default.xml", INDEX);
		*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	}
	
	//Create a new pit stop strategy object;
	strategy = new SimpleStrategy2();
	//init the fuel 
	strategy->setFuelAtRaceStart(t, carParmHandle, situation, INDEX);

	//Load and set parameters.
	//TBH Im not exactly sure what this does, or what the number means. but it gives a float.  
	//It seems given a DEFINED, factor as the third arg, will return a value of it. In this case MU(term often means Co-efficent of friction)
	MU_FACTOR = GfParmGetNum(*carParmHandle, MYBOT_SECT_PRIV, MYBOT_ATT_MUFACTOR, (char*)NULL, 0.68f);
}

//Start a new Race.
//This is for setting up the driver at the beginning of a race. 
void Driver::newRace(tCarElt * car, tSituation * situation)
{
	float deltaTime = (float)RCM_MAX_DT_ROBOTS; //Not sure what deltaTime actually is, I think it is time to make a decision for the bot. 
	MAX_UNSTUCK_COUNT = int(UNSTUCK_TIME_LIMIT / deltaTime);
	OVERTAKE_OFFSET_INC = OVERTAKE_OFFSET_SPEED*deltaTime;
	stuck = 0;
	alone = 1;
	clutchtime = 0.0f;
	oldlookahead = 0.0f;
	this->car = car;
	CARMASS = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0f);
	myoffset = 0.0f;
	initCa();
	initCw();
	initTireMu();
	initTCLfilter();

	// Create just one instance of cardata shared by all drivers.
	if (cardata == NULL || cardata == nullptr) {
		cardata = new Cardata(situation);
	}
	mycardata = cardata->findCar(car);
	currentsimtime = situation->currentTime;

	// initialize the list of opponents.
	opponents = new Opponents(situation, this, cardata);
	opponent = opponents->getOpponentPtr();

	// Set team mate.
	const char *teammate = GfParmGetStr(car->_carHandle, MYBOT_SECT_PRIV, MYBOT_ATT_TEAMMATE, NULL);
	if (teammate != NULL || teammate == nullptr) {
		opponents->setTeamMate(teammate);
	}

	// Initialize radius of segments.
	radius = new float[track->nseg];
	computeRadius(radius);

	learn = new SegLearn(track, situation, INDEX);

	// create the pit object.
	pit = new Pit(situation, this);

	//Initialize the Cherry learner	
	double* AA = setAvailActions();
	angle = trackangle - car->_yaw;
	oldstate = state = setState();
	EnterTheCherry(AA, ActSize, AAsz, state, stateSize);

	lastLap = car->race.laps;
	oldSeg = car->pub.trkPos.seg->id;
}

//Use this for determining what your Available actions are, this function returns a double[], where an action is size n, subset of the array
double* Driver::setAvailActions()
{
	//Actions will be in order of Throttle, Steer, negative values imply the opposite direction.
	AAsz = 18;//FullThrot,HalfThrot,NoThrot,FullBrake, Paired With Left,Right,NoTurn
	ActSize = 2;//Throttle steer, 
	double gofast[3] = { 0.35,0,-1 };//, half, no throttle, full brake
	double turn[3] = { 1, 0 , -1 };//straight Left right 
	AvailActions = new double[AAsz];
	int pos = 0;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			AvailActions[pos++] = gofast[i];
			AvailActions[pos++] = turn[j];
		}
	}
	action[0] = AvailActions[0];
	action[1] = AvailActions[1];
	return AvailActions;
}

//Will be used for setting the Start State and also for updating what the current state is.
double* Driver::setState()
{
	stateSize = 11;
	float tm = fabs(car->_trkPos.toMiddle);//absolute distance from middle, 
	float w = car->pub.trkPos.seg->width / WIDTHDIV;// how far from middle we will allow
	
	double* ret = new double[stateSize];
	int i = 0;


	//TODO: Look into setting up ego centric learners that learn different information based off whether we are in corner or not.

	//std::cout << "Width Percent = " << (int)(car->_trkPos.toMiddle / (car->pub.trkPos.seg->width / WIDTHDIV) * 100) / 10 << std::endl;
	//EgoStates
	double EgoDiscretize = 10, AloDiscretize = 100;
	
	ret[i++] = (car->pub.trkPos.seg->next->type);
	ret[i++] = 0;
	ret[i++] = 0;
	ret[i++] = (double)((int)(car->_speed_X)); std::cout << "EgoSpeed = " << ret[i - 1] << std::endl;
	ret[i++] = (double)((int)((tm / w) * EgoDiscretize)) / EgoDiscretize; std::cout << "track percent = " << ret[i - 1] << std::endl;
	ret[i++] = (int)(angle * 180 / 22.5); std::cout << "Ego Angle = " << ret[i - 1] << std::endl;// EgoDiscretize)) / EgoDiscretize;

	//Alo States
	ret[i++] = (car->pub.trkPos.seg->id);
	ret[i++] = (int)(getDistToSegeEnd()*AloDiscretize / AloDiscretize);	//std::cout << "percent of segend: " << ret[i - 1] << std::endl;
	ret[i++] = (double)((int)(car->_speed_X * AloDiscretize))/ AloDiscretize;// std::cout << "speed_X: " << ret[i - 1] << std::endl;
	ret[i++] = (double)((int)(angle  * AloDiscretize))/ AloDiscretize; //std::cout << "angle: " << ret[i - 1] << std::endl;
	ret[i++] = (double)((int)((tm / w) *AloDiscretize))/ AloDiscretize;// std::cout << "tm / w: " << ret[i - 1] << std::endl;



	return ret;//Remember to change the Cherries and and stateSize(top of this function)
};



//Drive during race. 
//Essential we check if the car is stuck, if not the we continue to drive with the results of getGear, getSteer etc...
void Driver::drive(tSituation * situation)
{
	memset(&car->ctrl, 0, sizeof(tCarCtrl));// we are allocation sizeof(tCarCtrl) bytes of memory from car->ctrl
	

	//For the driver
	trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
	angle = trackangle - car->_yaw;
	NORM_PI_PI(angle);

	if (isStuck()) {
		//delayUpdate = 150;
		getUnstuck();
	}

	else {
		{
			state = setState();
			bool doUpdate = false;
			for(int i =0;i<stateSize;i++)
			if (state[i] != oldstate[i] || firstStep || sqrt(car->_speed_X * car->_speed_X) < 1)
				doUpdate = true;
			if (doUpdate)
			{
				update(situation);
				oldstate = state;

				action = CherryAction();
				firstStep = false;
			}
		}
		{
			//std::cout << action[0] << ';' << action[1] << std::endl << std::endl;
			//Steering will be the indexed 1 value, the throttle will be the indexed 0 value
			car->_steerCmd = filterSColl(action[1]);
			//car->_steerCmd = filterSColl(action[1]);
			car->_gearCmd = getGear();
			//car->_brakeCmd = filterABS(filterBrakeSpeed(filterBColl(filterBPit(getBrake()))));

			if (action[0] < 0)
			{
				car->_brakeCmd = filterABS(filterBrakeSpeed(filterBColl(filterBPit(-1 * (float)action[0]))));
				car->_accelCmd = 0.0f;
			}
			else
			{
				car->_brakeCmd = 0.0f;
				car->_accelCmd = filterTCL(action[0]);
			}
			car->_clutchCmd = getClutch();
		}
	}
}

// Set pitstop commands.
int Driver::pitCommand(tSituation *s)
{
	car->_pitRepair = strategy->pitRepair(car, s);
	car->_pitFuel = strategy->pitRefuel(car, s);
	// This should be the only place where the pit stop is set to false!
	pit->setPitstop(false);
	return ROB_PIT_IM; // return immediately.
}

// End of the current race.
void Driver::endRace(tSituation *s)
{
	// Nothing for now.
}


/***************************************************************************
*
* utility functions
*
***************************************************************************/

//Handles Opening TheCherries DLL, and then grabs the Functions from the DLL
void Driver::DoDatDLL()
{

	LPCSTR txt = TEXT(".\\drivers\\mr_cherry\\TheCherries\\TheCherries.dll");
	TheCherriesDLLHandle = LoadLibrary(txt);
	if (TheCherriesDLLHandle != NULL)
	{
		std::cout << "Loaded DLL" << txt << "  " << std::endl;

		//Resolve the function addresses here
		CherryAction = (PerformAction)GetProcAddress(TheCherriesDLLHandle, "PerformAction");
		if (!CherryAction)//If it is null
		{
			std::cout << "FailedLoading Perform Action (CherryAction) function. (Line 235 of mr_cherry::driver)" << std::endl; abort();
		}
		else
			std::cout << "We know how to make a Cherry Act CherryAction" << std::endl;

		EnterTheCherry = (cherryEntry)GetProcAddress((TheCherriesDLLHandle), "cherryEntry");
		if (!EnterTheCherry)//If it is null
		{
			std::cout << "FailedLoading entryCherry (EnterTheCherry) function. (Line 243 of mr_cherry::driver)" << std::endl; abort();
		}
		else
			std::cout << "We know How to enter the Cherry! (EnterTheCherry)" << std::endl;


		CherryUpdate = (PerformUpdate)GetProcAddress(TheCherriesDLLHandle, "PerformUpdate");
		if (!CherryUpdate)
		{
			std::cout << "FailedLoading entryCherry (CherryUpdate) function. (Line 252 of mr_cherry::driver)" << std::endl; abort();
		}
		else
			std::cout << "We know How to craft a Cherry! (CherryUpdate)" << std::endl;

		return;
	}
	else
	{
		std::cout << "ERROR Loading DLL" << txt << "  " << GetLastError() << std::endl;
		system("PAUSE");
		assert((false));
	}
}

//Determines if we should start using unstuck protocols
bool Driver::isStuck()
{
	if (fabs(angle) > MAX_UNSTUCK_ANGLE &&
		car->_speed_x < MAX_UNSTUCK_SPEED &&
		fabs(car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) {
		if (stuck > MAX_UNSTUCK_COUNT && car->_trkPos.toMiddle*angle < 0.0) {
			return true;
		}
		else {
			stuck++;
			return false;
		}
	}
	else {
		stuck = 0;
		return false;
	}
}

//This function works to get the car unstuck from where it is currently stuck.
void Driver::getUnstuck()
{

	car->ctrl.steer = -angle / car->_steerLock;
	car->ctrl.gear = -1; // reverse gear
	car->ctrl.accelCmd = 0.5; // 50% accelerator pedal
	car->ctrl.brakeCmd = 0.0; // no brakes
}


void Driver::update(tSituation * s)
{
	//ret[i++] = (int)(car->_trkPos.toMiddle / (car->pub.trkPos.seg->width / WIDTHDIV) * 100);

	//For the Cherries
	double reward = -1 + (int)car->_speed_X - abs((int)(car->_trkPos.toMiddle / (car->pub.trkPos.seg->width / WIDTHDIV) * 100)) - abs((int)(angle * 180 / 22.5));
	//std::cout << "Updates" << std::endl;
	
	int curSeg = car->pub.trkPos.seg->id;

	if (car->race.laps > lastLap)
	{
		commitLapTimeWasTrue = true;
		lastLap = car->race.laps;
		oldSeg = -1;
		if (car->race.bestLapTime != 0 && car->race.bestLapTime < bestLapTime)
		{
			bestLapTime = car->race.bestLapTime;
			reward += 100;
		}
		else if ( car->race.bestLapTime < bestLapTime)
			reward += 1000;	
	}

	if (curSeg > oldSeg)
	{
		reward += 10;
		oldSeg = curSeg;
		
	}
	else if (curSeg < oldSeg)
	{
		reward -= 1000;
		oldSeg = curSeg;
	} 
	if(!car->race.commitBestLapTime && commitLapTimeWasTrue)//If we are not committing the last laptime, and we havent punished it yet.
	{
		commitLapTimeWasTrue = false;
		reward -= 1000;

	}
	if (car->_speed_X < 1 && action[0] < 0.1)
	{
		updatesWhileStopped+=2;
		reward -= 100;
	}

	
	//Detect off track
	float tm = fabs(car->_trkPos.toMiddle);//absolute distance from middle, 
	float w = car->pub.trkPos.seg->width / WIDTHDIV;// how far from middle we will allow
	if (tm > w && !offTrack )
	{
		offTrack = true;
		reward -= 1000;
	}
	else if(tm <= w) offTrack = false;
	
	 

			//std::cout << " Current Segment: " << car->pub.trkPos.seg->id << std::endl;
			//std::cout << state[0]<< state[1] <<state[2]<<state[3]<<state[4]<< std::endl;
			CherryUpdate(state, reward);
			return;
		
}

// Compute the allowed speed on a segment.
float Driver::getAllowedSpeed(tTrackSeg *segment)
{
	float mu = segment->surface->kFriction*TIREMU*MU_FACTOR;
	float r = radius[segment->id];
	float dr = learn->getRadius(segment);
	if (dr < 0.0f) {
		r += dr;
	}
	else {
		float tdr = dr*(1.0f - MIN(1.0f, fabs(myoffset)*2.0f / segment->width));
		r += tdr;
	}
	// README: the outcommented code is the more save version.
	/*if ((alone > 0 && fabs(myoffset) < USE_LEARNED_OFFSET_RANGE) ||
	dr < 0.0f
	) {
	r += dr;
	}*/
	r = MAX(1.0, r);

	return sqrt((mu*G*r) / (1.0f - MIN(1.0f, r*CA*mu / mass)));
}

// Compute fitting acceleration.
float Driver::getAccel()
{
	if (car->_gear > 0) {
		float allowedspeed = getAllowedSpeed(car->_trkPos.seg);
		if (allowedspeed > car->_speed_x + FULL_ACCEL_MARGIN) {
			return 1.0;
		}
		else {
			float gr = car->_gearRatio[car->_gear + car->_gearOffset];
			float rm = car->_enginerpmRedLine;
			return allowedspeed / car->_wheelRadius(REAR_RGT)*gr / rm;
		}
	}
	else {
		return 1.0;
	}
}

// Compute the length to the end of the segment.
float Driver::getDistToSegeEnd()
{
	if (car->_trkPos.seg->type == TR_STR) {
		return car->_trkPos.seg->length - car->_trkPos.toStart;
	}
	else {
		return (car->_trkPos.seg->arc - car->_trkPos.toStart)*car->_trkPos.seg->radius;
	}
}

// Compute initial brake value.
float Driver::getBrake()
{
	// Car drives backward?
	if (car->_speed_x < -MAX_UNSTUCK_SPEED) {
		// Yes, brake.
		return 1.0;
	}
	else {
	
		// We drive forward, normal braking.
		tTrackSeg *segptr = car->_trkPos.seg;
		float mu = segptr->surface->kFriction;
		float maxlookaheaddist = currentspeedsqr / (2.0f*mu*G);
		float lookaheaddist = getDistToSegeEnd();

		float allowedspeed = getAllowedSpeed(segptr);
		if (allowedspeed < car->_speed_x) {
			return MIN(1.0f, (car->_speed_x - allowedspeed) / (FULL_ACCEL_MARGIN));
		}

		segptr = segptr->next;
		while (lookaheaddist < maxlookaheaddist) {
			allowedspeed = getAllowedSpeed(segptr);
			if (allowedspeed < car->_speed_x) {
				if (brakedist(allowedspeed, mu) > lookaheaddist) {
					return 1.0f;
				}
			}
			lookaheaddist += segptr->length;
			segptr = segptr->next;
		}
		return 0.0f;
	}
}

// Compute gear. Transmission gear choice, would be based off of wheelspeed and engine rpms
int Driver::getGear()
{
	if (car->_gear <= 0) {
		return 1;
	}
	float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
	float omega = car->_enginerpmRedLine / gr_up;
	float wr = car->_wheelRadius(2);

	if (omega*wr*SHIFT < car->_speed_x) {
		return car->_gear + 1;
	}
	else {
		float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
		omega = car->_enginerpmRedLine / gr_down;
		if (car->_gear > 1 && omega*wr*SHIFT > car->_speed_x + SHIFT_MARGIN) {
			return car->_gear - 1;
		}
	}
	return car->_gear;
}

// Compute steer value.
float Driver::getSteer()
{
	float targetAngle;
	vec2f target = getTargetPoint();

	targetAngle = atan2(target.y - car->_pos_Y, target.x - car->_pos_X);//Compute the angle needed from where we are to where we want to be.
	targetAngle -= car->_yaw;
	NORM_PI_PI(targetAngle);
	return targetAngle / car->_steerLock;
}

// Compute the clutch value.
float Driver::getClutch()
{
	if (car->_gear > 1) {
		clutchtime = 0.0f;
		return 0.0f;
	}
	else {
		float drpm = car->_enginerpm - car->_enginerpmRedLine / 2.0f;
		clutchtime = MIN(CLUTCH_FULL_MAX_TIME, clutchtime);
		float clutcht = (CLUTCH_FULL_MAX_TIME - clutchtime) / CLUTCH_FULL_MAX_TIME;
		if (car->_gear == 1 && car->_accelCmd > 0.0f) {
			clutchtime += (float)RCM_MAX_DT_ROBOTS;
		}

		if (drpm > 0) {
			float speedr;
			if (car->_gearCmd == 1) {
				// Compute corresponding speed to engine rpm.
				float omega = car->_enginerpmRedLine / car->_gearRatio[car->_gear + car->_gearOffset];
				float wr = car->_wheelRadius(2);
				speedr = (CLUTCH_SPEED + MAX(0.0f, car->_speed_x)) / fabs(wr*omega);
				float clutchr = MAX(0.0f, (1.0f - speedr*2.0f*drpm / car->_enginerpmRedLine));
				return MIN(clutcht, clutchr);
			}
			else {
				// For the reverse gear.
				clutchtime = 0.0f;
				return 0.0f;
			}
		}
		else {
			return clutcht;
		}
	}
}

// Compute target point for steering.
vec2f Driver::getTargetPoint()
{
	tTrackSeg *seg = car->_trkPos.seg;
	float lookahead;
	float length = getDistToSegeEnd();
	float offset = getOffset();

	if (pit->getInPit()) {
		// To stop in the pit we need special lookahead values.
		if (currentspeedsqr > pit->getSpeedlimitSqr()) {
			lookahead = PIT_LOOKAHEAD + car->_speed_x*LOOKAHEAD_FACTOR;
		}
		else {
			lookahead = PIT_LOOKAHEAD;
		}
	}
	else {
		// Usual lookahead.
		lookahead = LOOKAHEAD_CONST + car->_speed_x*LOOKAHEAD_FACTOR;
		// Prevent "snap back" of lookahead on harsh braking.
		float cmplookahead = oldlookahead - car->_speed_x*RCM_MAX_DT_ROBOTS;
		if (lookahead < cmplookahead) {
			lookahead = cmplookahead;
		}
	}

	oldlookahead = lookahead;

	// Search for the segment containing the target point.
	while (length < lookahead) {
		seg = seg->next;
		length += seg->length;
	}

	length = lookahead - length + seg->length;
	float fromstart = seg->lgfromstart;
	fromstart += length;

	// Compute the target point.
	offset = myoffset = pit->getPitOffset(offset, fromstart);

	vec2f s;
	s.x = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x) / 2.0f;
	s.y = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y) / 2.0f;

	if (seg->type == TR_STR) {
		vec2f d, n;
		n.x = (seg->vertex[TR_EL].x - seg->vertex[TR_ER].x) / seg->length;
		n.y = (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y) / seg->length;
		n.normalize();
		d.x = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / seg->length;
		d.y = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / seg->length;
		return s + d*length + offset*n;
	}
	else {
		vec2f c, n;
		c.x = seg->center.x;
		c.y = seg->center.y;
		float arc = length / seg->radius;
		float arcsign = (seg->type == TR_RGT) ? -1.0f : 1.0f;
		arc = arc*arcsign;
		s = s.rotate(c, arc);

		n = c - s;
		n.normalize();
		return s + arcsign*offset*n;
	}
}

// Compute offset to normal target point for overtaking or let pass an opponent.
float Driver::getOffset()
{
	int i;
	float catchdist, mincatchdist = FLT_MAX, mindist = -1000.0f;
	Opponent *o = NULL;

	// Increment speed dependent.
	float incfactor = MAX_INC_FACTOR - MIN(fabs(car->_speed_x) / MAX_INC_FACTOR, (MAX_INC_FACTOR - 1.0f));

	// Let overlap or let less damaged team mate pass.
	for (i = 0; i < opponents->getNOpponents(); i++) {
		// Let the teammate with less damage overtake to use slipstreaming.
		// The position change happens when the damage difference is greater than
		// TEAM_DAMAGE_CHANGE_LEAD.
		if (((opponent[i].getState() & OPP_LETPASS) && !opponent[i].isTeamMate()) ||
			(opponent[i].isTeamMate() && (car->_dammage - opponent[i].getDamage() > TEAM_DAMAGE_CHANGE_LEAD) &&
			(opponent[i].getDistance() > -TEAM_REAR_DIST) && (opponent[i].getDistance() < -car->_dimension_x) &&
				car->race.laps == opponent[i].getCarPtr()->race.laps))
		{
			// Behind, larger distances are smaller ("more negative").
			if (opponent[i].getDistance() > mindist) {
				mindist = opponent[i].getDistance();
				o = &opponent[i];
			}
		}
	}

	if (o != NULL) {
		tCarElt *ocar = o->getCarPtr();
		float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
		float w = car->_trkPos.seg->width / WIDTHDIV - BORDER_OVERTAKE_MARGIN;
		if (side > 0.0f) {
			if (myoffset < w) {
				myoffset += OVERTAKE_OFFSET_INC*incfactor;
			}
		}
		else {
			if (myoffset > -w) {
				myoffset -= OVERTAKE_OFFSET_INC*incfactor;
			}
		}
		return myoffset;
	}


	// Overtake.
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if ((opponent[i].getState() & OPP_FRONT) &&
			!(opponent[i].isTeamMate() && car->race.laps <= opponent[i].getCarPtr()->race.laps))
		{
			catchdist = MIN(opponent[i].getCatchDist(), opponent[i].getDistance()*CATCH_FACTOR);
			if (catchdist < mincatchdist) {
				mincatchdist = catchdist;
				o = &opponent[i];
			}
		}
	}

	if (o != NULL) {
		// Compute the width around the middle which we can use for overtaking.
		float w = o->getCarPtr()->_trkPos.seg->width / WIDTHDIV - BORDER_OVERTAKE_MARGIN;
		// Compute the opponents distance to the middle.
		float otm = o->getCarPtr()->_trkPos.toMiddle;
		// Define the with of the middle range.
		float wm = o->getCarPtr()->_trkPos.seg->width*CENTERDIV;

		if (otm > wm && myoffset > -w) {
			myoffset -= OVERTAKE_OFFSET_INC*incfactor;
		}
		else if (otm < -wm && myoffset < w) {
			myoffset += OVERTAKE_OFFSET_INC*incfactor;
		}
		else {
			// If the opponent is near the middle we try to move the offset toward
			// the inside of the expected turn.
			// Try to find out the characteristic of the track up to catchdist.
			tTrackSeg *seg = car->_trkPos.seg;
			float length = getDistToSegeEnd();
			float oldlen, seglen = length;
			float lenright = 0.0f, lenleft = 0.0f;
			mincatchdist = MIN(mincatchdist, DISTCUTOFF);

			do {
				switch (seg->type) {
				case TR_LFT:
					lenleft += seglen;
					break;
				case TR_RGT:
					lenright += seglen;
					break;
				default:
					// Do nothing.
					break;
				}
				seg = seg->next;
				seglen = seg->length;
				oldlen = length;
				length += seglen;
			} while (oldlen < mincatchdist);

			// If we are on a straight look for the next turn.
			if (lenleft == 0.0f && lenright == 0.0f) {
				while (seg->type == TR_STR) {
					seg = seg->next;
				}
				// Assume: left or right if not straight.
				if (seg->type == TR_LFT) {
					lenleft = 1.0f;
				}
				else {
					lenright = 1.0f;
				}
			}

			// Because we are inside we can go to the border.
			float maxoff = (o->getCarPtr()->_trkPos.seg->width - car->_dimension_y) / 2.0 - BORDER_OVERTAKE_MARGIN;
			if (lenleft > lenright) {
				if (myoffset < maxoff) {
					myoffset += OVERTAKE_OFFSET_INC*incfactor;
				}
			}
			else {
				if (myoffset > -maxoff) {
					myoffset -= OVERTAKE_OFFSET_INC*incfactor;
				}
			}
		}
	}
	else {
		// There is no opponent to overtake, so the offset goes slowly back to zero.
		if (myoffset > OVERTAKE_OFFSET_INC) {
			myoffset -= OVERTAKE_OFFSET_INC;
		}
		else if (myoffset < -OVERTAKE_OFFSET_INC) {
			myoffset += OVERTAKE_OFFSET_INC;
		}
		else {
			myoffset = 0.0f;
		}
	}

	return myoffset;
}

// Compute the needed distance to brake.
float Driver::brakedist(float allowedspeed, float mu)
{
	float c = mu*G;
	float d = (CA*mu + CW) / mass;
	float v1sqr = currentspeedsqr;
	float v2sqr = allowedspeed*allowedspeed;
	return -log((c + v2sqr*d) / (c + v1sqr*d)) / (2.0f*d);
}


// If we get lapped reduce accelerator.
float Driver::filterOverlap(float accel)
{
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & OPP_LETPASS) {
			return MIN(accel, 0.5f);
		}
	}
	return accel;
}

// Brake filter for collision avoidance.
float Driver::filterBColl(float brake)
{
	float mu = car->_trkPos.seg->surface->kFriction;
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & OPP_COLL) {
			if (brakedist(opponent[i].getSpeed(), mu) > opponent[i].getDistance()) {
				return 1.0f;
			}
		}
	}
	return brake;
}
// Antilocking filter for brakes.
float Driver::filterABS(float brake)
{
	if (car->_speed_x < ABS_MINSPEED) return brake;
	int i;
	float slip = 0.0f;
	for (i = 0; i < 4; i++) {
		slip += car->_wheelSpinVel(i) * car->_wheelRadius(i);
	}
	slip = car->_speed_x - slip / 4.0f;
	if (slip > ABS_SLIP) {
		brake = brake - MIN(brake, (slip - ABS_SLIP) / ABS_RANGE);
	}
	return brake;
}

// Brake filter for pit stop.
float Driver::filterBPit(float brake)
{
	if (pit->getPitstop() && !pit->getInPit()) {
		float dl, dw;
		RtDistToPit(car, track, &dl, &dw);
		if (dl < PIT_BRAKE_AHEAD) {
			float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
			if (brakedist(0.0f, mu) > dl) {
				return 1.0f;
			}
		}
	}

	if (pit->getInPit()) {
		float s = pit->toSplineCoord(car->_distFromStartLine);
		// Pit entry.
		if (pit->getPitstop()) {
			float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
			if (s < pit->getNPitStart()) {
				// Brake to pit speed limit.
				float dist = pit->getNPitStart() - s;
				if (brakedist(pit->getSpeedlimit(), mu) > dist) {
					return 1.0f;
				}
			}
			else {
				// Hold speed limit.
				if (currentspeedsqr > pit->getSpeedlimitSqr()) {
					return pit->getSpeedLimitBrake(currentspeedsqr);
				}
			}
			// Brake into pit (speed limit 0.0 to stop)
			float dist = pit->getNPitLoc() - s;
			if (pit->isTimeout(dist)) {
				pit->setPitstop(false);
				return 0.0f;
			}
			else {
				if (brakedist(0.0f, mu) > dist) {
					return 1.0f;
				}
				else if (s > pit->getNPitLoc()) {
					// Stop in the pit.
					return 1.0f;
				}
			}
		}
		else {
			// Pit exit.
			if (s < pit->getNPitEnd()) {
				// Pit speed limit.
				if (currentspeedsqr > pit->getSpeedlimitSqr()) {
					return pit->getSpeedLimitBrake(currentspeedsqr);
				}
			}
		}
	}

	return brake;
}


// Reduces the brake value such that it fits the speed (more downforce -> more braking).
float Driver::filterBrakeSpeed(float brake)
{
	float weight = (CARMASS + car->_fuel)*G;
	float maxForce = weight + CA*MAX_SPEED*MAX_SPEED;
	float force = weight + CA*currentspeedsqr;
	return brake*force / maxForce;
}

//Not Implemented
float Driver::filterTurnSpeed(float brake)
{
	return 0.0f;
}

// TCL filter for accelerator pedal.
float Driver::filterTCL(float accel)
{
	float slip = (this->*GET_DRIVEN_WHEEL_SPEED)() - car->_speed_x;
	if (slip > TCL_SLIP) {
		accel = accel - MIN(accel, (slip - TCL_SLIP) / TCL_RANGE);
	}
	return accel;
}



// Hold car on the track.
float Driver::filterTrk(float accel)
{
	tTrackSeg* seg = car->_trkPos.seg;

	if (car->_speed_x < MAX_UNSTUCK_SPEED ||		// Too slow.
		pit->getInPit() ||							// Pit stop.
		car->_trkPos.toMiddle*speedangle > 0.0f)	// Speedvector points to the inside of the turn.
	{
		return accel;
	}

	if (seg->type == TR_STR) {
		float tm = fabs(car->_trkPos.toMiddle);
		float w = (seg->width - car->_dimension_y) / 2.0f;
		if (tm > w) {
			return 0.0f;
		}
		else {
			return accel;
		}
	}
	else {
		float sign = (seg->type == TR_RGT) ? -1.0f : 1.0f;
		if (car->_trkPos.toMiddle*sign > 0.0f) {
			return accel;
		}
		else {
			float tm = fabs(car->_trkPos.toMiddle);
			float w = seg->width / WIDTHDIV;
			if (tm > w) {
				return 0.0f;
			}
			else {
				return accel;
			}
		}
	}
}

// Steer filter for collision avoidance.
float Driver::filterSColl(float steer)
{
	int i;
	float sidedist = 0.0f, fsidedist = 0.0f, minsidedist = FLT_MAX;
	Opponent *o = NULL;

	// Get the index of the nearest car (o).
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & OPP_SIDE) {
			sidedist = opponent[i].getSideDist();
			fsidedist = fabs(sidedist);
			if (fsidedist < minsidedist) {
				minsidedist = fsidedist;
				o = &opponent[i];
			}
		}
	}

	// If there is another car handle the situation.
	if (o != NULL) {
		float d = fsidedist - o->getWidth();
		// Near, so we need to look at it.
		if (d < SIDECOLL_MARGIN) {
			/* compute angle between cars */
			tCarElt *ocar = o->getCarPtr();
			float diffangle = ocar->_yaw - car->_yaw;
			NORM_PI_PI(diffangle);
			// We are near and heading toward the car.
			if (diffangle*o->getSideDist() < 0.0f) {
				const float c = SIDECOLL_MARGIN / 2.0f;
				d = d - c;
				if (d < 0.0f) {
					d = 0.0f;
				}

				// Steer delta required to drive parallel to the opponent.
				float psteer = diffangle / car->_steerLock;
				myoffset = car->_trkPos.toMiddle;

				// Limit myoffset to suitable limits.
				float w = o->getCarPtr()->_trkPos.seg->width / WIDTHDIV - BORDER_OVERTAKE_MARGIN;
				if (fabs(myoffset) > w) {
					myoffset = (myoffset > 0.0f) ? w : -w;
				}

				// On straights the car near to the middle can correct more, in turns the car inside
				// the turn does (because if you leave the track on the turn "inside" you will skid
				// back to the track.
				if (car->_trkPos.seg->type == TR_STR) {
					if (fabs(car->_trkPos.toMiddle) > fabs(ocar->_trkPos.toMiddle)) {
						// Its me, I do correct not that much.
						psteer = steer*(d / c) + 1.5f*psteer*(1.0f - d / c);
					}
					else {
						// Its the opponent, so I correct more.
						psteer = steer*(d / c) + 2.0f*psteer*(1.0f - d / c);
					}
				}
				else {
					// Who is outside, heavy corrections are less dangerous
					// if you drive near the middle of the track.
					float outside = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
					float sign = (car->_trkPos.seg->type == TR_RGT) ? 1.0f : -1.0f;
					if (outside*sign > 0.0f) {
						psteer = steer*(d / c) + 1.5f*psteer*(1.0f - d / c);
					}
					else {
						psteer = steer*(d / c) + 2.0f*psteer*(1.0f - d / c);
					}
				}

				if (psteer*steer > 0.0f && fabs(steer) > fabs(psteer)) {
					return steer;
				}
				else {
					return psteer;
				}
			}
		}
	}
	return steer;
}

// TCL filter plugin for rear wheel driven cars.
float Driver::filterTCL_RWD()
{
	return (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) * car->_wheelRadius(REAR_LFT) / 2.0f;
}

// TCL filter plugin for front wheel driven cars.
float Driver::filterTCL_FWD()
{
	return (car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
		car->_wheelRadius(FRNT_LFT) / 2.0f;
}

// TCL filter plugin for all wheel driven cars.
float Driver::filterTCL_4WD()
{
	return ((car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
		car->_wheelRadius(FRNT_LFT) +
		(car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
		car->_wheelRadius(REAR_LFT)) / 4.0f;
}

// Traction Control (TCL) setup.
void Driver::initTCLfilter()
{
	const char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_RWD;
	}
	else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_FWD;
	}
	else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_4WD;
	}
}

// Compute aerodynamic downforce coefficient CA.
void Driver::initCa()
{
	const char *WheelSect[4] = { SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL };
	float rearwingarea = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*)NULL, 0.0f);
	float rearwingangle = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*)NULL, 0.0f);
	float wingca = 1.23f*rearwingarea*sin(rearwingangle);

	float cl = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*)NULL, 0.0f) +
		GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*)NULL, 0.0f);
	float h = 0.0f;
	int i;
	for (i = 0; i < 4; i++)
		h += GfParmGetNum(car->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*)NULL, 0.20f);
	h *= 1.5f; h = h*h; h = h*h; h = 2.0f * exp(-3.0f*h);
	CA = h*cl + 4.0f*wingca;
}


// Compute aerodynamic drag coefficient CW.
void Driver::initCw()
{
	float cx = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*)NULL, 0.0f);
	float frontarea = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*)NULL, 0.0f);
	CW = 0.645f*cx*frontarea;
}


// Init the friction coefficient of the the tires.
void Driver::initTireMu()
{
	const char *WheelSect[4] = { SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL };
	float tm = FLT_MAX;
	int i;

	for (i = 0; i < 4; i++) {
		tm = MIN(tm, GfParmGetNum(car->_carHandle, WheelSect[i], PRM_MU, (char*)NULL, 1.0f));
	}
	TIREMU = tm;
}


void Driver::computeRadius(float * radius)
{
	float lastturnarc = 0.0f;
	int lastsegtype = TR_STR;

	tTrackSeg *currentseg, *startseg = track->seg;
	currentseg = startseg;

	do {
		if (currentseg->type == TR_STR) {
			lastsegtype = TR_STR;
			radius[currentseg->id] = FLT_MAX;
		}
		else {
			if (currentseg->type != lastsegtype) {
				float arc = 0.0f;
				tTrackSeg *s = currentseg;
				lastsegtype = currentseg->type;

				while (s->type == lastsegtype && arc < PI / 2.0f) {
					arc += s->arc;
					s = s->next;
				}
				lastturnarc = arc / (PI / 2.0f);
			}
			radius[currentseg->id] = (currentseg->radius + currentseg->width / 2.0) / lastturnarc;
		}
		currentseg = currentseg->next;
	} while (currentseg != startseg);
}


int Driver::isAlone()
{
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & (OPP_COLL | OPP_LETPASS)) {
			return 0;	// Not alone.
		}
	}
	return 1;	// Alone.
}
