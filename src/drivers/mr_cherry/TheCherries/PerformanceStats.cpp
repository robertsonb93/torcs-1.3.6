#include "stdafx.h"
#include "PerformanceStats.h"


PerformanceStats::PerformanceStats()
	:cumulativeReward(0), modelAccesses(0), modelUpdates(0)
{
	stepsToGoal.push_back(0);
}

PerformanceStats::PerformanceStats(const double cmRwrd, const int modAcc, const int modUp)
	:cumulativeReward(cmRwrd),modelAccesses(modAcc),modelUpdates(modUp)
{
	stepsToGoal.push_back(0);
}


PerformanceStats::~PerformanceStats()
{
}


//Maintains a vector<int>, whenever the goal is reached, will start a new count from 0 
void PerformanceStats::TallyStepsToGoal(bool goalReached)
{
	if (goalReached)
		stepsToGoal.push_back(0);
	else
		stepsToGoal[stepsToGoal.size() - 1]++;	

}

//SetterFunction
void PerformanceStats::SetCumulativeReward(const double n)
{
	cumulativeReward = n;
}

void PerformanceStats::SetModelAccesses(const int n)
{
	modelAccesses = n;
}

void PerformanceStats::SetModelUpdates(const int n)
{
	modelUpdates = n;
}


//Getter Functions
double PerformanceStats::GetCumulativeReward() 
{
	return cumulativeReward;
}

int PerformanceStats::GetModelAccesses()
{
	return modelAccesses;
}

int PerformanceStats::GetModelUpdates()
{
	return modelUpdates;
}

//Will default to returning the entire vector of steps,
//Otherwise will return subvector of from indexStart to indexEnd
//Will force an exceeded end to the size-1
//Will force a preceeded start to the start
//A end that preceeds the begin throws an exception
//A start that exceeds the size-1 throws an exception
//A end that preceeds the start throws an exception
vector<double> PerformanceStats::GetStepsToGoal(int indexStart, int indexEnd)
{
	if (indexEnd == indexStart && indexStart == -1)//Default value will return the entire vector
	{
		return stepsToGoal;
	}
	if (indexEnd < 0)
		throw("Bad end index, preceeds 0 in PerformanceStats::GetStepsToGoal()");

	if (indexEnd >= (int)stepsToGoal.size())
		indexEnd = stepsToGoal.size() - 1;	
	
	if (indexStart < 0)
		indexStart = 0;
	
	if (indexStart >= (int)stepsToGoal.size())
		throw("Bad start index, exceeds or equals  PerformanceStats::GetStepsToGoal()::stepsToGoal::size()");

	if (indexEnd < indexStart)
		throw("indexEnd < indexStart in  PerformanceStats::GetStepsToGoal() ");

	vector<double>::const_iterator first = stepsToGoal.begin() + indexStart;
	vector<double>::const_iterator last = stepsToGoal.begin() + indexEnd;
	vector<double> newvec(first, last);
	return newvec;
}
