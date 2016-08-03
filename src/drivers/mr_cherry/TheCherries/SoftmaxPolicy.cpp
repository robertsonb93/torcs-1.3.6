#include "stdafx.h"
#include "SoftmaxPolicy.h"



SoftmaxPolicy::SoftmaxPolicy()
{
	srand(static_cast<unsigned int>(time(NULL) ));//Seeds the random generator to the time
													//The static_cast is not needed, but removes compiler warnings
}

SoftmaxPolicy::SoftmaxPolicy(double diffDefault)//For a different default T value for the temperature
	: defaultT(diffDefault)
{
	srand( static_cast<unsigned int>(time(NULL)) );
}


SoftmaxPolicy::~SoftmaxPolicy()
{
}

//Takes a vector of actions, a vector of values, and a possible single parameter in a vector (vector for polymorphism)
//will use the param as the temperature if it is available
//establishes probabilities on each value, then will check if they are greater then the threshold
//The first one greater then the threshold is used, if non are found then the last action is defaulted to
std::vector<double> SoftmaxPolicy::selectAction(std::vector<std::vector<double>>& availableActions, std::vector<double>& values, std::vector<double>& params)
{
	double temperature = defaultT;
	//if (!params.empty())//A different temperature is desired
	//	temperature = params[0];

	std::vector<double> probs;
	probs.resize(values.size());//Keep atleast n amount for the vector

	//fill in the probability of every given value
	probs[0] = exp(values[0] / temperature);
	for (unsigned int i = 1; i < values.size(); i++)
		probs[i] = probs[i - 1] + exp(values[i] / temperature);
	
	//need a number between 0-1
	double threshold = ((double)rand() / RAND_MAX);
	threshold *= probs[probs.size() - 1];

	//Check which probs exceed the threshold
	for (unsigned int i = 0; i < probs.size(); i++)
	{
		if (probs[i] > threshold)
			return availableActions[i];
	}
	return availableActions[availableActions.size() - 1];//defualt to last if no probs are high enough
}

//will set the DefaultT to the given parameter
//Will return true if the parameter is viable and accepted
//will return false otherwise and set DefaultT to 1.
bool SoftmaxPolicy::setDefaultT(double newT)
{
	if (newT <= 1 && newT > 0)
	{
		defaultT = newT;
		return true;
	}
	defaultT = 1;
	return false;
}
