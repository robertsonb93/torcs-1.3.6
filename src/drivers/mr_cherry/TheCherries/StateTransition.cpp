#include "stdafx.h"
#include "StateTransition.h"



StateTransition::StateTransition(const vector<double>& OS, const vector<double>& NS,const vector<double>& act,double rwrd, bool SessionHasEnded)
	: oldState(OS), newState(NS), reward(rwrd), action(act), absorbingStateReached(SessionHasEnded) //This is MEMBER INIT LIST, to set the consts
{};


StateTransition::~StateTransition()//No pointers are used
{
}

//Getters
vector<double> StateTransition::getOldState() const { return oldState; } ;
vector<double> StateTransition::getNewState() const { return newState; };
vector<double> StateTransition::getAction() const { return action; };
double StateTransition::getReward() const { return reward; } ;
