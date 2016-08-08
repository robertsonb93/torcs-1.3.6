#include "stdafx.h"
#include "AgentSingle.h"
#include "ModelBasedEgoAlo.h"
#include "EpsilonGreedy.h"
#include "OptimalPolicy.h"
#include <iostream>

	AgentBase* Agent = nullptr;
	vector<double> StartStateVect;
	vector<double> LastAction;
	vector<vector<double>> AvailableActions;
	 int ActionSz;
	 int StateSz;



// Module entry point.
//This Agent system is different from the Original Cherries Project, 
//TThe Agent No longer is the center, where it answers to the Interface, and gives actions and receives immediate rewards from the World,
//The Agent no longer has a world or an interpretor, as the Interface and World are the Same object, (Likely mr_cherry), this would cause a Circular logic flaw for update and performing action
extern "C" int _cdecl cherryEntry(const double* AvailActionsInput, const int ActionSize, const int AASize, const double* StartState, const int SSSize)
{
	for (int i = 0; i < AASize; )//This is all the off the individual actions, we must parse them into groups off Actionsize
	{
		//vector<double> Action((AvailActionsInput+i* ActionSize), (AvailActionsInput + i* ActionSize) + ActionSize); //Create a Vector of AvailActions
		vector<double> Action;
		for (int j = 0; j < ActionSize; j++)
		{
			Action.push_back(AvailActionsInput[i + j]);
		}
		i += ActionSize;		
		AvailableActions.push_back(Action);
	}

	//for each (auto act in AvailableActions)
	//{
	//	for each (auto a in act)
	//	{
	//		cout << a << ',';
	//	}
	//	cout << endl;
	//}


	StartStateVect.assign(StartState, StartState + SSSize);

	Agent = new AgentSingle();
	Agent->setActionValue(new ModelBasedLearning(AvailableActions, StartStateVect));
	Agent->setPolicy(new EpsilonGreedy(0.98));
	Agent->setPossibleActions(AvailableActions);
	ActionSz = ActionSize;
	StateSz = SSSize;
	Agent->SetState(StartStateVect);

	return 0;
}


//Will call up the Agents SelectAction(),
//For consideration, look into giving the agent what its current state is as well and using setState, I'm not sure if will become out of date from
//What the drivers state is. 
extern "C" double*  PerformAction()
{
	 LastAction = (Agent->SelectAction());

	double* outArray = new double[ActionSz];
	for (int i = 0; i < ActionSz; i++)
	{

		outArray[i] = LastAction[i];
	}

	return outArray;
}


//After performing the Action from the Agent, The world must find what the new state is in that its at, give it and a reward to the agent for the action
extern "C" void PerformUpdate(const double* StatePrime, const double Reward)
{
	//This function must produce a stateTransition object for the Agent to log
	vector<double> SPrime(StatePrime, StatePrime + StateSz);
	//std::cout << Reward << endl;
	

	Agent->LogEvent(StateTransition(Agent->GetState(), SPrime, LastAction, Reward));
}