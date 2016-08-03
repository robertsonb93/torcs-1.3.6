#include "stdafx.h"
#include "EpsilonGreedy.h"



EpsilonGreedy::EpsilonGreedy()
{
	srand(static_cast<unsigned int> (time(NULL)));
}

EpsilonGreedy::EpsilonGreedy(double defE)
{
	defaultE = defE;
	srand ( static_cast<unsigned int> (time(NULL)) );

}


EpsilonGreedy::~EpsilonGreedy()
{
}

//Has a random chance of randomly selecting an action, 
//Otherwise selects the optimal action
std::vector<double> EpsilonGreedy::selectAction(std::vector<std::vector<double>>& availableActions, std::vector<double>& values, std::vector<double>& params)
{
	double randVal = ((double)rand()/RAND_MAX); // put the value between 0-1
	
	if (randVal < defaultE)//If the value is below epsilon, then use the optimal action
	{
		auto end = values.end();
		int i = 1, c = 0;
		auto it = values.begin();
		for (auto newIt = values.begin() + 1; newIt != end; ++newIt, i++)
		{
			if (*it < *newIt)
			{
				c = i;
				it = newIt;
			}
		}
		return availableActions[c];
	}
	else //Randomly select a value
	{
		return availableActions[(unsigned int)rand() % availableActions.size()];
	}
}

//returns True if the value was changed to the specific parameter
//Return false if the value given was not accepted, sets epsilon to 0.8
bool EpsilonGreedy::setEpsilon(double newEps)
{
	if (newEps < 1 && newEps >= 0)
	{
		defaultE = newEps;
		return true;
	}

	defaultE = 0.8;
	return false;
}
