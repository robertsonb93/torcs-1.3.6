#pragma once
#ifndef EGOALOMAZE_H
#define EGOALOMAZE_H

#include "stdafx.h"
#include "GridWorldBase.h"



class EgoAloMaze :
	public GridWorldBase
{
public:
	enum stateTypes { openSpace = 0, wall, lava, goal, agentLocation, visitedOpenSpace, visitedWall, visitedLava, visitedGoal };

	EgoAloMaze(int numActions = 4);
	~EgoAloMaze();


	//Accessed by interface/form		
	mazeType ShowState();
	void Load(mazeType inMap, vector<double>& start);
	void ChangeRwrds(double newOpen, double newWall, double newLava, double newGoal);
	int GetMap(int x, int y);//Returns the type of tile on the map at the spec position
	mazeType GetMap();

	//Accessed by the Agent/Interpretor 
	StateTransition PerformStep(const vector<double>& Action, const vector<double>& state, PerformanceStats& perfStats);
	vector<vector<double>> GetVisitedStates();
	vector<vector<double>> GetAvailableActions();
	vector<double> GetStartState();


private:
	int viewRange = 1;
	vector<double> GetEgo(int y, int x);
	void SetActions();
	void SeenState(vector<double> state);


	//Members//
	vector<double> startState, aloStartState;
	
	vector<vector<double>> availableActions;
	vector<vector<double>> visitedStates; //Each internal vector is a state
	vector<vector<double>> currLocations; //Will use the seen transitions to try and mark where agents currently are.
	int numActions;

	double openSpaceRwrd = -0.01;
	double wallRwrd = -0.1;
	double lavaRwrd = -0.5;
	double goalRwrd = 10;
};

#endif