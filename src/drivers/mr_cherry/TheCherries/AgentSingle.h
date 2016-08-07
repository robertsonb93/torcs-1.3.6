#pragma once
#ifndef _AGENTSINGLE_H
#define _AGENTSINGLE_H

#include "stdafx.h"
#include "WorldBase.h"
#include "AgentBase.h"
#include "QLearning.h"
#include "PolicyHeaders.h"

	

typedef OptimalPolicy _DefaultPolicyType;
typedef QLearning _DefaultAVType;

	//Derived class for creating a single Agent in the world
	//
	class  AgentSingle : public AgentBase
	{
	public:
		 AgentSingle();
		 ~AgentSingle();

		 vector<double> SelectAction();
		 vector<double> SelectAction(vector<double>& params);
		 void LogEvent(StateTransition transition);
		 void SaveLearnerArchive(string filename);
		 void LoadLearnerArchive(string filename);

		 //Used when Creating the Agent, you must call these functions afterwards as there will not be any action value or world for the agent yet.
		 void setActionValue(ActionValue* newAv);
		 void setPolicy(PolicyBase* pol);
		 WorldBase* setInterpretor(InterpretorBase* Intrptr);//Return a pointer to the world that the interpreter creates.

		 void setPossibleActions(vector < vector<double>>& AA) { possibleActions = AA; };
		 void SetState(vector<double>& inputState);
		 vector<double> GetState();
		 PerformanceStats GetStats();
		 void SetStats(PerformanceStats& PS);

	private:
		vector<double> state;//Where is the agent currently located
		PolicyBase* policy = nullptr;


		//****Understand that the following are owned by the base class Agent.cpp****//
		//ActionValue* actionValue;
		//InterpretorBase* interpreter
		//std::vector<vector<double>> possibleActions;
	};

#endif