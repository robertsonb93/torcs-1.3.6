#pragma once
#ifndef _WORLDBASE_H
#define _WORLDBASE_H

//Will be the abstract Base class for the worlds, Abstract base class is used as a type of interface
//Declaring a function as:
//  virtual func() = 0; 
//Makes an abstract base class of WorldBase, and furthermore, forces derived classed to define func(), and is
//now called a pure virtual function
#include "stdafx.h"
#include "PerformanceStats.h"
#include "AgentBase.h"


	using namespace std;
	class  WorldBase abstract
	{
	public:

		WorldBase() {};
		virtual ~WorldBase() = 0 {};
		virtual StateTransition PerformStep(const vector<double>&, const vector<double>&,PerformanceStats&) = 0;

	};


#endif