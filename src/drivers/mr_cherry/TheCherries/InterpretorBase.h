#pragma once
#ifndef INTERPRETORBASE_H
#define INTERPRETORBASE_H

#include "stdafx.h"

class WorldBase;
//A Base class for any future Interpretor Objects, Interpretors are intended to be owned by an Agent and have a world which they interpret
//There job is to be the communicator between the world and the Agent, so will retrieve the state and translate it to something the agent can use
//It will also be used to give the agents command to world and get the reward.



class InterpretorBase
{
public:
	InterpretorBase() {};
	~InterpretorBase() { };

	virtual StateTransition DoAction(const vector<double>& Action,const vector<double>& state, PerformanceStats& perfstats) = 0;
	virtual WorldBase* GetWorld() = 0;

	//Getters to get info from the world for the agent. 
	virtual vector<vector<double>> GetVisitedStates() = 0;
	virtual vector<vector<double>> GetAvailableActions() = 0;
	virtual vector<double> GetStartState() =0;


};
#endif
