#include "stdafx.h"
#include "AgentSingle.h"
#include "ModelBasedEgoAlo.h"
#include "EpsilonGreedy.h"
#include "OptimalPolicy.h"
#include <iostream>

//The outermost vector on these refer to the index of a specific agent in a multi agent system.
	vector<AgentBase*> AgentVec; //	AgentBase* Agent = nullptr;

	vector<vector<double>> StartStateVect;
	vector<vector<double>> LastAction;
	vector<vector<vector<double>>> AvailableActions;
	vector<int> ActionSz;
	vector<int> StateSz;
	int numAgents =0;
	vector<string> modelNames;


// Module entry point.
//This Agent system is different from the Original Cherries Project, 
//TThe Agent No longer is the center, where it answers to the Interface, and gives actions and receives immediate rewards from the World,
//The Agent no longer has a world or an interpretor, as the Interface and World are the Same object, (Likely mr_cherry), this would cause a Circular logic flaw for update and performing action
extern "C" int _cdecl cherryEntry(const double* AvailActionsInput, const int ActionSize, const int AASize, const double* StartState, const int SSSize)
{
	int curAgent = numAgents++;
	AvailableActions.resize(numAgents);
	StartStateVect.resize(numAgents);
	LastAction.resize(numAgents);

	ActionSz.push_back(ActionSize);
	StateSz.push_back(SSSize);

	for (int i = 0; i < AASize; i += ActionSize )//This is all the off the individual actions, we must parse them into groups off Actionsize
	{
		vector<double> Action;
		for (int j = 0; j < ActionSize; j++)
		{
			Action.push_back(AvailActionsInput[i + j]);
		}
		AvailableActions[curAgent].push_back(Action);
	}


	StartStateVect[curAgent].assign(StartState, StartState + SSSize);//Give the Agent its starting State
	AgentVec.push_back(new AgentSingle());
	//Agent->setActionValue(new ModelBasedEgoAlo(AvailableActions, StartStateVect, 6));
	//Agent->setActionValue(new ModelBasedEgoAlo(AvailableActions, StartStateVect, 6,1,0.99999,120));
	AgentVec[curAgent]->setActionValue(new ModelBasedLearning(AvailableActions[curAgent], StartStateVect[curAgent], 10000,0.,10000));
	AgentVec[curAgent]->setPolicy(new EpsilonGreedy(.98));
	AgentVec[curAgent]->setPossibleActions(AvailableActions[curAgent]);

	
	AgentVec[curAgent]->SetState(StartStateVect[curAgent]);


	return 0;
}

extern "C" void _cdecl cherryExit()
{
	for (int i = numAgents-1; i >=0; i--)
		if (AgentVec[i] != nullptr)
		{
			delete AgentVec[i];
			AgentVec[i] = nullptr;
		}

}

//Used for naming the cherry for saving and loading the model.
extern "C"  void _cdecl nameCherry(const int desiredCherry, const char* name, const int nameSize)
{
	if (modelNames.size() <= desiredCherry)
		modelNames.resize(desiredCherry+1);

	modelNames[desiredCherry] = string (name);
}

//Will call up the Agents SelectAction(),
//For consideration, look into giving the agent what its current state is as well and using setState, I'm not sure if will become out of date from
//What the drivers state is. 
extern "C" double*  PerformAction(const int chosenAgent)
{
	 LastAction[chosenAgent] = (AgentVec[chosenAgent]->SelectAction());

	double* outArray = new double[ActionSz[chosenAgent]];

	for (int i = 0; i < ActionSz[chosenAgent]; i++)
		outArray[i] = LastAction[chosenAgent][i];
	
	
	return outArray;
}


//After performing the Action from the Agent, The world must find what the new state is in that its at, give it and a reward to the agent for the action
extern "C" void PerformUpdate(const int chosenAgent, const double* StatePrime, const double Reward)
{
	
	//This function must produce a stateTransition object for the Agent to log
	vector<double> SPrime(StatePrime, StatePrime + StateSz[chosenAgent]);
	
	AgentVec[chosenAgent]->LogEvent(StateTransition(AgentVec[chosenAgent]->GetState(), SPrime, LastAction[chosenAgent], Reward));

}

extern "C" void LoadLearner()
{
	
	//Check that the names match the AgentVecSize which would match the numAgent counter;
	if (numAgents != AgentVec.size()){cout << "NumAgent != AgentVecSize. " << numAgents << " != " << AgentVec.size(); abort();}
	if (modelNames.size() != AgentVec.size()){cout << "modelNamesSize != AgentVecSize. " << modelNames.size() << " != " << AgentVec.size(); abort();}
	string filename = "drivers\\mr_cherry\\TheCherries\\";

	for (int i = 0; i < AgentVec.size(); i++)
	{
		AgentVec[i]->LoadLearnerArchive(filename + modelNames[i] + ".bin");
		
	}

	
}

extern "C" void SaveLearner()
{

	
	if (numAgents != AgentVec.size()){cout << "NumAgent != AgentVecSize. " << numAgents << " != " << AgentVec.size(); abort();}
	if (modelNames.size() != AgentVec.size()){cout << "modelNamesSize != AgentVecSize. " << modelNames.size() << " != " << AgentVec.size(); abort();}

	string filename = "drivers\\mr_cherry\\TheCherries\\";
	for (int i = 0; i < AgentVec.size(); i++)
	{
		AgentVec[i]->SaveLearnerArchive(filename + modelNames[i]+ ".bin");
	
	}

}
