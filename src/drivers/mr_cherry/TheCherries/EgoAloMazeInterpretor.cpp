#include "stdafx.h"
#include "EgoAloMazeInterpretor.h"



EgoAloMazeInterpretor::EgoAloMazeInterpretor()
{
	world = EgoAloMaze();
}

EgoAloMazeInterpretor::EgoAloMazeInterpretor(int numActions)
{
	world = EgoAloMaze(numActions);
}


EgoAloMazeInterpretor::~EgoAloMazeInterpretor()
{
}


//Do ACtion will need the current agents state, and will also need an action to perform from said state
//It will likely be called by the Agent, which in turn is called to do action by the form/Interface
//It will return the resulting Transition that the world says occurred. 
StateTransition EgoAloMazeInterpretor::DoAction(const vector<double>& action, const vector<double>& state, PerformanceStats& perfStats)
{
	return world.PerformStep(action, state, perfStats);//This will already have the ego and alo information incorporated within.
}