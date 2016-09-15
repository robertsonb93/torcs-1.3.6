#pragma once
#ifndef MODELBASEDEGOALO_H
#define MODELBASEDEGOALO_H

#include "stdafx.h"
#include "QLearning.h"
#include "ModelBasedLearning.h"
#include "ModelBasedBase.h"



class ModelBasedEgoAlo :
	public ModelBasedBase
{
private:

	typedef vector<double>  stateType;
	typedef vector<double> actionType;
	typedef unordered_map<stateType, map<actionType, map<stateType, pair<double, double>>>>  SASTable;
	typedef unordered_map<stateType, map<actionType, double>> QTableType;

public:
	
	ModelBasedEgoAlo() : egoSize(NULL) {};
	ModelBasedEgoAlo(const vector<vector<double>>& AvailActions, const vector<double>& StartState,const int NumEgoFeatures);
	ModelBasedEgoAlo(const vector<vector<double>>& AvailActions, const vector<double>& StartState, const int NumEgoFeatures, double DefQ, double gam, int maxUps);
	virtual ~ModelBasedEgoAlo();


	
	stateType PredictNextState(stateType state, actionType action);
	map<stateType, double> PredictNextStates(stateType state, actionType action);
	double PredictReward(stateType state, actionType action, stateType newState);

	//Brunt of the Learner, Used for getting actions and making updates to/from the learners
	double Update(const StateTransition & transition);
	double SetQValues(const std::vector<double>& state, const std::vector<vector<double>>& actions, const int Qval);

	double Value(const vector<double>& state, const vector<double>& action);
	vector<double> Value(const vector<double>& state, const vector<vector<double>>& actions);
	void ResetAllocentric();



	//Get/Set the Performance Stats
	PerformanceStats& GetStats();
	void SetStats(PerformanceStats& PS);
	void ResetStats();

private:
	
	void updatePredictionModels(const stateType& oldEgo,const actionType& act,const stateType& newAlo,const stateType& oldAlo,const double rew);


	//Serialization
	friend class boost::serialization::access;
	template<class Archive>
	inline void serialize(Archive & ar, const unsigned int version);

	//****MEMBERS****//
	ModelBasedLearning aloLearner,rewardPredictionModel;
	vector<ModelBasedLearning> aloFeaturePredictionModels; //This is the Ego part of the learner, each model uses .value to return its prediction on what change in feature is

	//Kept for re-setting the model-Based(alocentric) learner
	vector<vector<double>> availableActions;
	vector<double> startState; 
	
	vector<unordered_map<stateType, int>> visitedStates; //Keep track #-times an action has led to an alo-state. 
	map<actionType,unordered_map<stateType, int>> visitedEgoStates;//Keep Track #-times an action has been seen at an ego state
	PerformanceStats perfStats;

	int updateTerminationStepCount = 10;
	 int egoSize;
	double gamma = 0.9, defQ = 10, maxUps = 120;
	int steps = 0,minStepsRequired = 15;//How many steps this class has seen taken, (number of times that update has been called)

};
#endif

template<class Archive>
inline void ModelBasedEgoAlo::serialize(Archive & ar, const unsigned int version)//Does not provide the start state, parameters (except egoSize)
{
	ar & boost::serialization::base_object<ModelBasedBase>(*this);
	ar & availableActions;
	ar & startState;
	ar & rewardPredictionModel;
	ar & aloFeaturePredictionModels;
	ar & aloLearner;
	ar & visitedEgoStates;
	ar & visitedStates;
	ar & egoSize;
	ar & gamma;
	ar & defQ;
	ar & maxUps;
	ar & steps;
	ar & minStepsRequired;

}

BOOST_CLASS_EXPORT_KEY(ModelBasedEgoAlo);