#pragma once
#ifndef _MODELBASEDLEARNING_H
#define _MODELBASEDLEARNING_H


#include "stdafx.h"
#include "ModelBasedBase.h"



class ModelBasedLearning : virtual public ModelBasedBase
{
private:
	//Keeping the typedefs private makes it so that other classes can nicely define there own and there
	//is no confusion of the definition of the type. Otherwise other classes could call up stateType etc.. and
	//it could reference this define.
	typedef vector<double>  stateType;
	typedef vector<double> actionType;
	typedef unordered_map<stateType, map<actionType, map<stateType, pair<double,double>>>>  SASTable;
	typedef unordered_map<stateType, map<actionType, double>> QTableType;

public:
	
	ModelBasedLearning() {};
	ModelBasedLearning(const vector<vector<double>>& AvailActions, const vector<double>& StartState );
	ModelBasedLearning(const vector<vector<double>>& AvailActions, const vector<double>& StartState, double DefQ, double gam, int maxUps);
	virtual ~ModelBasedLearning();

	map<stateType,double> PredictNextStates(stateType state, actionType action);
	stateType PredictNextState(stateType state, actionType action);
	double PredictReward(stateType state, actionType action, stateType newState);
	map<double, double> PredictReward(stateType state, actionType action);

	//Running Functions, used for the brunt of the learner
	double Value(const vector<double>& state,const vector<double>& action);
	vector<double> Value(const vector<double>& state,const vector<vector<double>>& actions);
	double Update(const StateTransition & transition);
	
	PerformanceStats& GetStats();
	void SetStats(PerformanceStats& PS);
	void ResetStats();

	//friend class boost::serialization::access;

private:

	void UpdateQ(const stateType& state, actionType& action);
	void cleanUpPriority();
	map<vector<double>, double>::iterator findMaxPriority();
	double calcUpdate_value(const stateType& iterMax);

private:

	//Serialization
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version);

	/******MEMBERS******/
	
	

	double defaultQ = 10, gamma = 0.95;
	int c = 1;
	int maxUpdates = 360;
	SASTable TR;//[state][action].first is the T: [state][action].second is the R
	QTableType QTable;
	unordered_map<stateType, map<stateType, unordered_set<vector<double>>>> predecessors;//double<vector> = actiontype
	map<stateType, double> priority;
	
	PerformanceStats stats;
	vector<actionType> availableActions;
	stateType startState;
	map<actionType, double> defaultMap;//Used for creating a quick table of default values for the Qtable
};

template<class Archive>
inline void ModelBasedLearning::serialize(Archive & ar, const unsigned int version)
{
	cerr << "ENTERED MODEL BASED LEARNING SERIALIZE" << endl;
	//ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ActionValue);
	//ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModelBasedBase);
	ar & TR;
	ar & QTable;
	ar & predecessors;
	ar & availableActions;
	cerr << " ModelBasedLearning Serialize Complete" << endl << endl;
}

BOOST_CLASS_EXPORT_KEY(ModelBasedLearning);

#endif