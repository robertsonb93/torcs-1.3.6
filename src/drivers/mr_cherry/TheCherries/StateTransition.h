#pragma once
#ifndef _STATETRANSITION_H
#define _STATETRANSITION_H

#include "stdafx.h"
//#include<vector>


	using namespace std;
	//Simple class for holding the properties of a transition
	//CONSIDER MAKING CLASS A SINGLETON?

	class StateTransition
	{
	public:
			StateTransition(const vector<double>& OS, const vector<double>& NS,const vector<double>& act, double rwrd, bool SessionHasEnded = false);
		 ~StateTransition();

		 vector<double> getOldState() const;
		vector<double>  getNewState() const ;
		 vector<double>  getAction() const ;
		 double getReward() const;

	private:
		const vector<double> oldState;
		const vector<double> newState;
		const vector<double> action;
		const double reward;
		const bool absorbingStateReached;
	};

#endif