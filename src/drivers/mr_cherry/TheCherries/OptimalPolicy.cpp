#include "stdafx.h"
#include "OptimalPolicy.h"



OptimalPolicy::OptimalPolicy()
{
}


OptimalPolicy::~OptimalPolicy()
{
	
}

//Will default to using the firstmost element in the case of a tie between values
//Will return a vector of doubles that represent the action
//Uses the Q-values in comparison to determine the optimal action
vector<double> OptimalPolicy::selectAction( std::vector<std::vector<double>>& availableActions, std::vector<double>& values, std::vector<double>& params)
{
	//vector<double> bestAction = availableActions[0];
	end = values.end();
	int i = 1, c=0;
	it = values.begin();
		for (newIt = values.begin()+1;newIt != end; ++newIt,i++)
		{
			if (*it < *newIt)
			{
				c = i;
				it = newIt;
			}
		}
	return availableActions[c];
}
