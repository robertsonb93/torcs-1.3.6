


#ifdef _WIN32
#include <windows.h>
#include <assert.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "driver.h"


#define NBBOTS 10


static const char* botname[NBBOTS] = {
	"mr_cherry 1", "mr_cherry 2", "mr_cherry 3", "mr_cherry 4", "mr_cherry 5",
	"mr_cherry 6", "mr_cherry 7", "mr_cherry 8", "mr_cherry 9", "mr_cherry 10"
};

static const char* botdesc[NBBOTS] = {
	"mr_cherry 1", "mr_cherry 2", "mr_cherry 3", "mr_cherry 4", "mr_cherry 5",
	"mr_cherry 6", "mr_cherry 7", "mr_cherry 8", "mr_cherry 9", "mr_cherry 10"
};

static  Driver *driver[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);




// Module entry point.
extern "C" int mr_cherry(tModInfo *modInfo)
{
		int i;

		// Clear all structures.
		memset(modInfo, 0, 10 * sizeof(tModInfo));

		for (i = 0; i < NBBOTS; i++)
		{
			modInfo[i].name = strdup(botname[i]);	// name of the module (short).
			modInfo[i].desc = strdup(botdesc[i]);	// Description of the module (can be long).
			modInfo[i].fctInit = InitFuncPt;			// Init function.
			modInfo[i].gfId = ROB_IDENT;				// Supported framework version.
			modInfo[i].index = i;						// Indices from 0 to 9.
		}
		

		return 0;

	}


// Module interface initialization.
static int InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;

	// Create robot instance for index.
	driver[index] = new Driver(index);
	itf->rbNewTrack = initTrack;	// Give the robot the track view called.
	itf->rbNewRace = newRace;		// Start a new race.
	itf->rbDrive = drive;		// Drive during race.
	itf->rbPitCmd = pitcmd;		// Pit commands.
	itf->rbEndRace = endRace;		// End of the current race.
	itf->rbShutdown = shutdown;		// Called before the module is unloaded.
	itf->index = index;		// Index used if multiple interfaces.
	return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	driver[index]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
	driver[index]->newRace(car, s);
}


// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
	driver[index]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	return driver[index]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s)
{
	driver[index]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index)
{
	delete driver[index];
}

