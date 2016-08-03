#include "stdafx.h"
#include "TraditionalMazeInterpretor.h"


TraditionalMazeInterpretor::TraditionalMazeInterpretor()
{
	world = TraditionalMaze();
}

TraditionalMazeInterpretor::TraditionalMazeInterpretor(int numActions)
{
	world = TraditionalMaze(numActions);
}


TraditionalMazeInterpretor::~TraditionalMazeInterpretor()
{
}


//Do ACtion will need the current agents state, and will also need an action to perform from said state
//It will likely be called by the Agent, which in turn is called to do action by the form/Interface
//It will return the resulting Transition that the world says occurred. 
StateTransition TraditionalMazeInterpretor::DoAction(const vector<double>& action, const vector<double>& state, PerformanceStats& perfStats)
{
	
	return world.PerformStep(action, state,perfStats);
}

