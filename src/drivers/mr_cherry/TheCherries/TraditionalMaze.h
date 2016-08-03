#pragma once
#ifndef _TRADITIONALMAZE_H
#define _TRADITIONALMAZE_H

#include "stdafx.h"
#include "GridWorldBase.h"
#include "PolicyHeaders.h"

//Class is derived from GridWorldBase and furthermore from WorldBase
//Uses a Bitmap to represent the maze
//Can use the <BMP> mapBMP member from GridworldBase
//Can use the <AgentBase*> agent from WorldBase

typedef AgentSingle _DefaultAgentType;

	class TraditionalMaze :
		public GridWorldBase
	{
	public:
		enum stateTypes { openSpace = 0, wall, lava, goal,agentLocation,visitedOpenSpace,visitedWall,visitedLava,visitedGoal };//This will auto assign wall =1, lava =2, ...;

		 TraditionalMaze(int numberOfActions = 4);
		 ~TraditionalMaze();

		 //Accessed by interface/form		
		 mazeType ShowState();
		 void Load(mazeType inMap, vector<double>& start);
		 void ChangeRwrds(double newOpen, double newWall, double newLava, double newGoal);
		 int GetMap(int x, int y);//Returns the type of tile on the map at the spec position
		 mazeType GetMap();

		 //Accessed by the Agent 
		 StateTransition PerformStep(const vector<double>& Action, const vector<double>& state, PerformanceStats& perfStats);
		 vector<vector<double>> GetVisitedStates();
		 vector<vector<double>> GetAvailableActions();
		 vector<double> GetStartState();
		

	private:

		void SetActions();
		void SeenState(vector<double> state);
		

		//Members//
		vector<double> startState;
		vector<vector<double>> availableActions;
		vector<vector<double>> visitedStates; //Each internal vector is a state
		vector<vector<double>> currLocations; //Will use the seen transitions to try and mark where agents currently are.
		int numActions;

		double openSpaceRwrd = -0.01;
		double wallRwrd = -0.1;
		double lavaRwrd = -0.5;
		double goalRwrd = 10;

		//OutputData OD;
		//DirectoryInfo DI;

	};

#endif