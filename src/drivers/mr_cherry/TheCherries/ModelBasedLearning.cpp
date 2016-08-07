
#include "stdafx.h"
#include "ModelBasedLearning.h"
//BOOST_CLASS_EXPORT(ModelBasedLearning);
BOOST_CLASS_EXPORT_IMPLEMENT(ModelBasedLearning)



ModelBasedLearning::ModelBasedLearning(const vector<vector<double>>& AvailActions, const vector<double>& StartState)
{
	availableActions = AvailActions;
	startState = StartState;
	for each (vector<double> act in AvailActions)
	{
		defaultMap[act] = defaultQ;
	}
	

}

ModelBasedLearning::ModelBasedLearning(const vector<vector<double>>& AvailActions, const vector<double>& StartState, double DefQ, double gam, int maxUps)
{
	availableActions = AvailActions;
	startState = StartState;
	defaultQ = DefQ;
	gamma = gam;
	maxUpdates = maxUps;

	for each (vector<double> act in AvailActions)
	{
		defaultMap[act] = defaultQ;
	}


}

ModelBasedLearning::~ModelBasedLearning()
{
}

//Returns a map<vector<double>,double> associated with a State prime(state to action results in state prime), 
//The keys are the state primes possible, and the values are: value/sumValues
//That is the values of any given SPrime is the percentage it will be reached.
//If the transition has not been seen before it will return a defaulted map, with no key-value pairs
map<vector<double>, double> ModelBasedLearning::PredictNextStates(stateType state, actionType action)
{
	stats.SetModelAccesses(stats.GetModelAccesses() + 1);

	map<stateType, double> response;
	double total = 0;
	auto TR_Begin = TR[state][action].begin();
	auto TR_End = TR[state][action].end();
	map<stateType, pair<double, double>>::iterator itr_cur;

	for (itr_cur = TR_Begin ; itr_cur != TR_End; ++itr_cur)
	{
		total += itr_cur->second.first;
	}

	//If ther is no map under state, or action then a new defaulted map with no entries will be created
	for (itr_cur = TR_Begin; itr_cur != TR_End; ++itr_cur)
	{
			response[itr_cur->first] = itr_cur->second.first / total;
	}
		return response;
}

//Returns a state prime prediction using the T table counts,
//to determine the most likely statePrime given a current state and action
vector<double> ModelBasedLearning::PredictNextState(stateType state, actionType action)
{
	stats.SetModelAccesses(stats.GetModelAccesses() + 1);
	stateType ret;
	int maxCount = -1;//Default for t table is at 0;
	auto TR_Begin = TR[state][action].begin();
	auto TR_End = TR[state][action].end();
	map<stateType, pair<double, double>>::iterator itr_cur;

	for (itr_cur = TR_Begin; itr_cur != TR_End; ++itr_cur)
	{
		if (itr_cur->second.first > maxCount)
			ret = itr_cur->first;
	}
	return ret;
}

double ModelBasedLearning::PredictReward(stateType state, actionType action, stateType newState)
{
	stats.SetModelAccesses(stats.GetModelAccesses() + 1);

	//Trying to emplace a new map into this. Will return the already in place value if its there.
	return (TR[state][action].emplace(pair<stateType, pair<double, double>>(newState, pair<double, double>(0, defaultQ)))).first->second.second;
}


//Currently Returns a map of all the associated possible rewards of the given action, 
//The Key-Value pair of the map will be 2 doubles,
//Key : the reward
//Value : occurance count of the Reward
map<double, double> ModelBasedLearning::PredictReward(stateType state, actionType action)
{
	map<double, double> ret;
	const auto & end = TR[state][action].end();
	for (auto itr_cur = TR[state][action].begin(); itr_cur != end; ++itr_cur)
	{
		try
		{//Look up the value in ret, and increment that value of if it is there
			ret.at(itr_cur->second.second)++;
		}
		catch (out_of_range)
		{//Initialize the non-existent number to one (we have seen it only once)
			ret[itr_cur->second.second] = 1;
		}
	}
	return ret;
}

//Evaluate what the value in the Qtable is, given a specific state and a specific action
double ModelBasedLearning::Value(const vector<double>& state, const vector<double>& action)
{
	auto statePtr = QTable.emplace(pair<stateType, map<vector<double>, double>>(state, defaultMap));//emplace will see if the Key exists, if not will emplace a defaultMap at it
	return statePtr.first->second[action];

}
//, will return a vector of rewards pertaining to the vector of actions from the state
vector<double> ModelBasedLearning::Value(const vector<double>& state, const vector<vector<double>>& actions)
{
	//size the vector from the start with default vals
	const auto & statePtr = (QTable.emplace( pair<stateType, map<vector<double>, double>>(state, defaultMap)));//emplace will see if the Key exists, if not will emplace a defaultMap at i
	size_t sz = actions.size();
	
	//if (statePtr.second)//boolean if defaultMap was emplaceed, if it was all the values will be defQ anyways
		//return vector<double>(sz,defaultQ);

	
	vector<double> ret(sz, defaultQ);
	for (size_t i = 0; i < sz; i++)
			ret[i] = statePtr.first->second[actions[i]];

		return ret;
}

//Creates an update for the modelbased learner,
//Takes a transition object that holds a state,action,stateprime,reward
//Use the transition to update the transition(T) table and
//Use the transition to update the Reward(R) table, 
//Then perform a prioritized sweep across the states that led to the current transition.
#include<iostream>
double ModelBasedLearning::Update(const StateTransition& transition)
{
	//Update the T,R tables, to have current Reward values, and times visited
	//Will need to make sure that the states and actions are in the T table, use .at for setting a default value
	stats.SetCumulativeReward(stats.GetCumulativeReward() + transition.getReward());

	stateType OS(transition.getOldState());
	stateType NS(transition.getNewState());
	actionType ACT(transition.getAction());

	auto & TR_it = TR[OS][ACT].emplace(pair<stateType, pair<double, double>>(NS, pair<double, double>(0, 0))).first->second;
	++(TR_it.first);//The trans
	TR_it.second = transition.getReward();

	//Update a predecessors so that they contain the new state. and the transition from the old
	predecessors[NS][OS].emplace(ACT);
	priority[OS]= (numeric_limits<double>::infinity());

	//Preform Prioritized sweeping, i is the return value of the function, and signifies the number of Updates completed
	int i = 0;
	for (i; i<maxUpdates; ++i)
	//while(true)
	{
		//Find the highest Priority State		
		const map<vector<double>, double>::iterator iterMax = findMaxPriority();
		
		if(iterMax == priority.end() || !(iterMax -> second))
			break;
		
		double valueChange = calcUpdate_value( (iterMax->first) );//Update the QTable at the given state(max Priority state)


		//lastly update the priorities before doing the loop again
		//Understand that iterPredecState is predecessor[PriorityState], thus points to a map<state,unordered_Set<actiontype>>
		//map<stateType, unordered_set<actionType>>::iterator iterPredecState(predecessors.find(iterMax->first)->second.begin());
		map<stateType, unordered_set<vector<double>>>::iterator iterPredecState(predecessors[iterMax->first].begin());
		const auto & end = predecessors[iterMax->first].end();
		double valueTrans;	
		pair<stateType, double> priorInsrt;
		
			while (iterPredecState != end )
			{
				priorInsrt = pair<stateType,double>(iterPredecState->first,0);
				for each  (actionType predact in iterPredecState->second)
				{
					valueTrans = (valueChange * TR[iterPredecState->first][predact][iterMax->first].first);

					auto pri = priority.emplace(priorInsrt);
					if (pri.first->second < valueTrans)
						pri.first->second = valueTrans;
				}
				++iterPredecState;
			}
			priority.erase(iterMax);
	}
	cleanUpPriority();
	return i;
}

PerformanceStats& ModelBasedLearning::GetStats()
{
	return stats;
}

void ModelBasedLearning::SetStats(PerformanceStats & PS)
{
	stats = PS;
}

void ModelBasedLearning::ResetStats()
{
	stats = PerformanceStats();
}


inline void ModelBasedLearning::UpdateQ(const stateType& state, actionType& action)
{
		//First add up the sum of the number of transitions that have occured from this state,action
	double P = 0;

	//Do a single lookup for the end and begin of the T table;
	map<stateType, pair<double, double>>* SAInterest = &TR[state][action];
		const map<stateType, pair<double,double>>::iterator& T_end = (*SAInterest).end();
		const map<stateType, pair<double,double>>::iterator& T_begin = (*SAInterest).begin();
		map<stateType, pair<double,double>>::iterator T_iter;//T_iter.first is a state, second.first is transitions, second.second is reward

	for (T_iter =T_begin; T_iter != T_end; ++T_iter)
	{
		P += T_iter->second.first;
	}
	if (!P)
		return;

	//Now ensure the QTable has the state in it, 
	//This will only do the states that the T table has seen. 
	double newQ = 0;
	double maxQ(numeric_limits<double>::lowest());
	pair<unordered_map<stateType, map<actionType, double>>::iterator, bool> QPtr;
	map<actionType, double>::iterator qEnd;
		
	//T_iter is the [S][A], so now we will iterate all the S primes,
	//We will see if the SPrime is in the QTable and put a defaultQ values at its position if it isnt. While grabbing an iterator
	for (T_iter = T_begin; T_iter != T_end; ++T_iter)
	{
	    QPtr = QTable.emplace( pair<stateType, map<vector<double>, double>>(T_iter->first, defaultMap));//emplace will see if the Key exists, if not will emplace a defaultMap at it
		if (!QPtr.second)//Will be false if the element already existed at iter->first, if it did, it will not overwrite
		{
			qEnd = QPtr.first->second.end();
			for(auto it = QPtr.first->second.begin();it != qEnd; ++it)
			{				
				if (it->second > maxQ)
					maxQ = it->second;
			}
		}
		else
			maxQ = defaultQ;

		double thisR = T_iter->second.second;//The reward, where T_iter is the [S][A], ->second is the S^, thus ->second.second is reward for [s][a][s^]
		newQ += (T_iter->second.first / P) * (thisR + (gamma * maxQ));
	}
		//Put the new value in the QTable
		QTable[state][action] = newQ;
		//cout << QTable.size() + QTable[state].size() << " Is Qtable size" << endl;

	}

//When the priority has a number of zero priorities, this will attempt to remove, to make faster iteration.
inline void ModelBasedLearning::cleanUpPriority()
{
	const auto & end(priority.end());
		for (map<vector<double>, double>::iterator curIter = priority.begin(); curIter != end;)
		{
			if (!(curIter->second))
			{
				priority.erase(curIter++);
			}
			else
			{
				++curIter;
			}
		}
	
}

//Simply iterate over the priority map and find what priority is currently the max
inline map<vector<double>, double>::iterator ModelBasedLearning::findMaxPriority()
{
	//map<vector<double>, double>::iterator iterMax = priority.begin(); //since we have to see all the elements of the map, iterator should be faster then look ups with []
	map<actionType,double>::iterator max = priority.begin();

	const auto & end(priority.end());
	for (map<vector<double>, double>::iterator curIter = priority.begin(); curIter != end; ++curIter)
	{
		if (curIter->second > max->second)
		{
			max = curIter;
		}
	}
	return max;
}

//Will take the maximum priority state, and run an update on it. this includes calling UpdateQ
//Afterwards will calculate what teh change in the QTable was for the iterMax state and return.
inline double ModelBasedLearning::calcUpdate_value(const stateType& state)
{
	//Perform Update step on the highest priority, currently pointed to by the iterMax
	vector<double> oldValues = Value(state, availableActions);

	int valSize = oldValues.size();
	auto ovIt = oldValues.begin();
	double maxOldValue = *ovIt++;
	for (; ovIt != oldValues.end();++ovIt)
	{
		if (maxOldValue < *ovIt)
			maxOldValue = *ovIt;
	}


	//Update the values currently held by the max priority state for each possible action
	auto end(availableActions.end());
	for (vector<actionType>::iterator iterCurAct = availableActions.begin(); iterCurAct != end; ++iterCurAct)
	{
		UpdateQ( state, *iterCurAct);	
	}
	stats.SetModelUpdates(stats.GetModelUpdates() + availableActions.size());

	//Find out what the values in the Q-table have been changed to,
	vector<double> newValues = Value(state, availableActions);
	ovIt = newValues.begin();
	double maxNewValue = *ovIt++;
	for (; ovIt != newValues.end(); ++ovIt)
	{
		if (maxNewValue < *ovIt)
			maxNewValue = *ovIt;
	}

	double valueChange = maxOldValue - maxNewValue;
	if (valueChange < 0)
		valueChange *= -1;

	return valueChange;
}


