#pragma once
#ifndef _HASHVECDOUBLES_H
#define _HASHVECDOUBLES_H

#include "stdafx.h"

using namespace std;

template<>
struct hash<vector<double>>
{
public:
	typedef vector<double> argument_type;
	typedef std::size_t  result_type;

	result_type operator()(const vector<double> & v) const
	{
		return GetHashForDoubleVector(v);
	}

	static int GetHashCodeForBytes(const char * bytes, int numBytes)
	{
		unsigned long h = 0, g;
		for (int i = 0; i<numBytes; i++)
		{
			h = (h << 4) + bytes[i];
			if (g = h & 0xF0000000L) { h ^= g >> 24; }
			h &= ~g;
		}
		return h;
	}

	static int GetHashForDouble(const double& v)
	{
		return GetHashCodeForBytes((const char *)&v, sizeof(v));
	}

	static int GetHashForDoubleVector(const vector<double> & v)
	{
		long ret = 0;
		int shift = 8 * 0;
		for (size_t i = 0; i < v.size(); i++)
		{
			ret += ((long)v[i]) << shift;
			shift = 8 * i + 1;
		}
			//ret += ((i + 1)*(GetHashForDouble(v[i])));
		return ret;
	}
};

#endif