#pragma once



#ifndef _SEGLEARN_H_
#define _SEGLEARN_H_



//I Need to do more reading into this class, But i believe it is for the driver to learn how to handle segments better.
//Could be a potential spot for the learning algs.

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>



class SegLearn {
public:
	SegLearn(tTrack* t, tSituation *s, int driverindex);
	~SegLearn();

	float getRadius(tTrackSeg *s) { return radius[s->id]; }
	void update(tSituation *s, tTrack *t, tCarElt *car, int alone, float offset, float outside, float *r);

private:
	float *radius;
	int *updateid;
	int nseg;

	bool check;
	float rmin;
	int lastturn;
	int prevtype;
	static const int BUFSIZE;
	char filename[1024];

	void writeKarma();
	FILE* getKarmaFilename(tTrack* t, tSituation *s, int driverindex);
	FILE* tryKarmaFilename(char* buffer, int size, const char *path, int driverindex, const char *tbuf, bool storelocalfilename);
	bool readKarma(tTrack* track, tSituation *s, float *radius, int *uid, int driverindex);
};


#endif