#include "stdafx.h"
#include "QLearning.h"
BOOST_CLASS_EXPORT_IMPLEMENT(QLearning)




//constructor
 QLearning::QLearning(const vector<vector<double>> AA)
{
	availableActions = AA;
	alpha = 0.9;
	gamma = 0.9;
	defaultQ = 0.1;

	for each(auto act in AA)
	defaultMap[act] = defaultQ;
};


 QLearning::QLearning(const vector<vector<double>> AA, const double alph, const double gam, const double defQ)
{
	alpha = alph;
	gamma = gam;
	defaultQ = defQ;
	availableActions = AA;

	for each(auto act in AA)
		defaultMap[act] = defaultQ;
}


//destructor
 QLearning::~QLearning()//No Pointers Created
{}

//Returns a single value associated with the value of the given Action and state
 double QLearning::Value(const vector<double>& state, const vector<double>& action) 
 {
	 //Emplace will check if the Key[state] exists, if it does not, it will make a default map. 
	 //Where the position is that the state is made/found, a pair<iter,bool> is created, so we reference .first "The iterator"
	 //From the iterator which references the state map, we just look up the action at ->second which is the map of actions to QValues
	return table.emplace(pair<vector<double>, map<vector<double>, double>>(state, defaultMap)).first->second[action];
 };


 //Takes the state vector and a vector of actions
 //returns a vector of values associated with each action at the state vector
vector<double> QLearning::Value(const vector<double>& state, const vector<vector<double>>& action)
{
	const auto & iter = table.emplace(pair<vector<double>, map<vector<double>, double>>(state, defaultMap));
		size_t sz = action.size();

	if (iter.second)//boolean if defaultMap was emplaceed, if it was all the values will be defQ anyways
		return vector<double>(sz, defaultQ);


	vector<double> ret(sz, defaultQ);
	for (size_t i = 0; i < sz; i++)
		ret[i] = iter.first->second[action[i]];

	return ret;
};


//Updates the transition from an old state to a new state in the q-tables.
//the action,oldstate,newstate,reward are all in the transition object, accessed by get functions
//
double QLearning::Update(const StateTransition& transition)
{

	stats.SetCumulativeReward(stats.GetCumulativeReward() + transition.getReward());

	vector<double> OS(transition.getOldState());
	vector<double> NS(transition.getNewState());
	vector<double> ACT(transition.getAction());

	//Need to make sure that the old state and the new state are both in the QTable
	 auto * OS_Ptr = &(table.emplace(pair<vector<double>, map<vector<double>, double>>(OS, defaultMap)).first->second[ACT]);//Grab pointer to save on look ups
	const auto & NS_Iter = table.emplace(pair<vector<double>, map<vector<double>, double>>(NS, defaultMap));//Want the Boolean on if we have seen the state before

	double q_s_a = *OS_Ptr;//Put the Value of the pointer into Q_S_A

	double maxQ(NS_Iter.first->second.begin()->second);	//Init to the first Q value., .begin() is an iter pointing at the first action,Qvalue pair
	if (!(NS_Iter.second)) //if the value was emplaced instead of just found, if emplaced all the vals are the same
	{	
		map<vector<double>, double>::iterator qEnd = NS_Iter.first->second.end();

		//Find the MaxQ value of the table[newstate] across all actions												 							
		for(auto it = NS_Iter.first->second.begin();it != qEnd; ++it)
		{
			{
				if (it->second > maxQ)
					maxQ = it->second;
			}
		}
	}
	
	*OS_Ptr = *OS_Ptr + alpha * (transition.getReward() + gamma * maxQ - *OS_Ptr);//The bellman equation 
	return abs(*OS_Ptr - q_s_a);
};

PerformanceStats& QLearning::GetStats() 
{
	return stats;
}

//Will take the given PerformanceStats and set the actionvalue stats to be the same
void QLearning::SetStats(PerformanceStats & PS)
{
	stats = PS;
};

