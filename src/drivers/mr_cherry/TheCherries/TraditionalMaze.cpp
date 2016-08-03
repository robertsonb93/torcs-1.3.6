
#include "stdafx.h"
#include "TraditionalMaze.h"
#include "AgentSingle.h"


//If numberOfActions is not 4 or 8 then it will default to 4
//4 represents the the 4 polar directions left,up,right,down
//8 represents the polars and the corners between, clockwise in representation
TraditionalMaze::TraditionalMaze(int numberOfActions )
	: numActions(numberOfActions)
{
	if (numActions != 4 && numActions != 8)
		numActions = 4;
	SetActions();
	startState.push_back(1);
	startState.push_back(1);
}

TraditionalMaze::~TraditionalMaze()
{
}

//Performs a step by the agent, 
//By-product, will also add any newly discovered states to the Vector of previously vistedState
//Parameters are the action desired be performed
//The state the agent is currently in
//The Perfstats that will be updated by the world, and returned back to the agent.
StateTransition TraditionalMaze::PerformStep(const vector<double>& action, const vector<double>& state,PerformanceStats& PerfStats)
{

	SeenState(state);//Will auto-check and add a state if it has not yet been seen

	vector<double> newState;
	double reward = 0;
	bool absorbingStateReached = false;
	
	//Get the type of the new location, i.e, wall, lava, blank, goal...
	 stateTypes newStateType = (stateTypes)(int)mapBMP[(int)state[0] + (int)action[0]][(int)state[1] + (int)action[1]];

	 //stateTypes is an enumeration 
	 switch (newStateType)
	 {
	 case openSpace: //Open space
	 {
		 newState.push_back(state[0] + action[0]);//Push the new y and then the new x
		 newState.push_back(state[1] + action[1]);
		 reward = openSpaceRwrd;
		 break;
	 }
	 case wall:
	 {
		 newState = state; // the state does not change
		 reward = wallRwrd;
		 break;
	 }
	 case lava:
	 {
		 newState.push_back(state[0] + action[0]);
		 newState.push_back(state[1] + action[1]);
		 reward = lavaRwrd;
		 break;
	 }
	 case goal:
	 {
		 newState = startState;
		 reward = goalRwrd;
		 absorbingStateReached = true;
		 break;
	 }
	 default: throw invalid_argument("unhandled/invalid enumerated stateType in TheCherries::TradMaze::PerformStep");
	 }//End Switch statement.

			
	  //Update where we currently see the agent(s)
		bool newStateSeen = true;

		for (int i = 0;i<currLocations.size();i++)
		{
			if (currLocations[i] == state)
			{
				currLocations[i][0] = newState[0];
				currLocations[i][1] = newState[1];
				newStateSeen = false;
			}
		}

		if (newStateSeen)
			currLocations.push_back(newState);//We are seeing a new agent, possibly one that was overlapped onto another. 
		
	
	PerfStats.TallyStepsToGoal(reward == goalRwrd);//Update the performance Stats
	return StateTransition(state, newState, action, reward, absorbingStateReached);//Return a new transition
}


//Need to show that states have been visited, and where the agent is currently located
//The agent location will be marked as a agentLoc in the map,
GridWorldBase::mazeType TraditionalMaze::ShowState()
{
	mazeType outMap = mapBMP;

	//Loop will mark visited areas in the map
	for each (vector<double> state in visitedStates)
	{
		int y = (int)state[0];
		int x = (int)state[1];
		switch ((int)outMap[y][x])
		{
		case openSpace: outMap[y][x] = visitedOpenSpace; break;
		case lava: outMap[y][x] = visitedLava; break;
		case goal: outMap[y][x] = visitedGoal; break;
		}

		for each (vector<double> agentLoc in currLocations)
		{
			outMap[(int)agentLoc[0]][(int)agentLoc[1]] = agentLocation;
		}
	}

	return outMap;
}


//Sets up the MapBMP inherited from GridworldBase
//The Form will have to provide each position, and the value for it(from enumerated types)
//Will have to make sure the Agent loads the start state from startState after this point.
void TraditionalMaze::Load(mazeType inMap, vector<double>& start)
{
	mapBMP = inMap;
	startState = start;
	mapBMP[(int)start[0]][(int)start[1]] = openSpace;//Notice marking the starting position as white with the map that is kept by TradMaze

	currLocations.push_back(vector<double> {start[0], start[1]});//We are pushing the the starting location as the agents currlocation
	
	visitedStates.clear();

}

//If it is decided that any of the goals should be replaced
//Then this function will change all the current rewards for this world.
void TraditionalMaze::ChangeRwrds(double newOpen, double newWall, double newLava, double newGoal)
{
	openSpaceRwrd = newOpen;
	wallRwrd = newWall;
	lavaRwrd = newLava;
	goalRwrd = newGoal;
}

int TraditionalMaze::GetMap(int x, int y)
{
	return (int)mapBMP[x][y];
}

//Function that defines what the available actions will be, if the constructor has num-actions set to anything
//besides 8, it will default to 4
void TraditionalMaze::SetActions()
{
	if (numActions == 4)
	{
		availableActions.resize(4);
		availableActions[0] = { 0, -1 };//Left
		availableActions[1] = { -1, 0 };//Up
		availableActions[2] = { 0, 1 };//Right
		availableActions[3] = { 1, 0 };//Down
	}
	if (numActions == 8)
	{
		availableActions.resize(8);

		availableActions[0] = { 0, -1 };//Left
		availableActions[1]=  { -1,-1 };//Left UP,
		availableActions[2] = { -1, 0 };//Up
		availableActions[3] = { -1,1 };//Right UP,
		availableActions[4] = { 0, 1 };//Right
		availableActions[5] = { 1,1 };//Right Down
		availableActions[6] = { 1, 0 };//Down
		availableActions[7] = { 1,-1 };//Left Down,
	}
}

vector<vector<double>> TraditionalMaze::GetMap()
{
	return mapBMP;
}

vector<vector<double>> TraditionalMaze::GetVisitedStates()
{
	return visitedStates;
}

vector<double> TraditionalMaze::GetStartState()
{
	return startState;
}

//Checks if a state has been previously visited by the agent. 
//If the agent has not previously seen, will push the state onto the 
//vector of visited states
void TraditionalMaze::SeenState(vector<double> state)
{
	bool seenBefore = false;
	for each  (vector<double> seenState in visitedStates)
	{
		if (seenState == state)
		{
			seenBefore = true;
			break;
		}
	}
	if (!seenBefore)
	{
		visitedStates.push_back(state);
	}
}

vector<vector<double>> TraditionalMaze::GetAvailableActions()
{
	return availableActions;
}

