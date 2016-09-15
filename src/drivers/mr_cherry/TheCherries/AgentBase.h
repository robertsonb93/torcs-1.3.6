#pragma once
#ifndef _AGENTBASE_H
#define _AGENTBASE_H
//Will function as the base class for the agent system.
//At the time of writing, the rational was that the agent system will need to be the communicator between the learning 
//algorithms and the world. 
//the derived agents are thought to be able to split into 2 groups, single agent class or multi-agent class
#include "stdafx.h"
#include "ActionValue.h"
#include "PolicyBase.h"
#include "InterpretorBase.h"


class WorldBase;
	//Base class for agents for polymorphisms
	class  AgentBase abstract
	{
	public:
		 AgentBase() {};
		 virtual ~AgentBase() = 0 {
			 delete actionValue; delete interpretor; actionValue = nullptr; interpretor = nullptr;
		 };

		 virtual std::vector<double> SelectAction() = 0;
		 virtual std::vector<double> SelectAction(vector<double>& params) = 0;//For use with policies that have parameters
		 virtual void LogEvent(StateTransition transition) = 0;
		 virtual PerformanceStats GetStats() = 0;
		 virtual void SaveLearnerArchive(string filename) = 0;
		 virtual void LoadLearnerArchive(string filename) = 0;

		 //Use when creating a new Agent, instantiates a world and actionValue for the agent to work in. 
		 virtual void setPossibleActions(vector<vector<double>>& AA) = 0;
		 virtual void setActionValue(ActionValue* newAv) = 0;
		 virtual void setPolicy(PolicyBase* newPol) = 0;
		 virtual void SetStateValue(const std::vector<double>& state, const std::vector<vector<double>>& actions, const int Qval) = 0;

		 
		 virtual WorldBase* setInterpretor(InterpretorBase* intrptr) = 0;//The type of intrptr will define the world.


		 virtual void SetStats(PerformanceStats& PS) = 0;
		 virtual void SetState(vector<double>& inputState) = 0;
		 virtual vector<double> GetState() = 0;
		 vector<vector<double>> GetAvailAction() { return possibleActions; };

		//Members//
	protected:
		//Having these members will force derived classes to have these members, but not tell them what to do with them.
		ActionValue* actionValue = nullptr;
		InterpretorBase* interpretor = nullptr;
		std::vector<std::vector<double>> possibleActions;
	};

#endif