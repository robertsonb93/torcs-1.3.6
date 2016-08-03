#pragma once



#ifndef _SPLINE_H_
#define _SPLINE_H_


class SplinePoint {
public:
	float x;	// x coordinate.
	float y;	// y coordinate.
	float s;	// slope.
};


class Spline {
public:
	Spline(int dim, SplinePoint *s);

	float evaluate(float z);

private:
	SplinePoint *s;
	int dim;
};

#endif // _SPLINE_H_

