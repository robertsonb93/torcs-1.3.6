#pragma once
#ifndef TRADMAZEINTERPRETOR_H
#define TRADMAZEINTERPRETOR_H


#include "stdafx.h"
#include "InterpretorBase.h"
#include "TraditionalMaze.h"



class TraditionalMazeInterpretor :
	public InterpretorBase
{
public:
	TraditionalMazeInterpretor();
	TraditionalMazeInterpretor(int numActions = 4);
	~TraditionalMazeInterpretor();

	StateTransition DoAction(const vector<double>& Action, const vector<double>& state,PerformanceStats& perfstat);
	WorldBase* GetWorld() { return &world; };//Returns a reference of the world so that any interface can look at the world

	//Getters to get info from the world for the agent. 
	vector<vector<double>> GetVisitedStates() { return world.GetVisitedStates(); };
	vector<vector<double>> GetAvailableActions() { return world.GetAvailableActions();};
	vector<double> GetStartState() { return world.GetStartState(); };


	//Members//
	TraditionalMaze world; 
	
};

#endif