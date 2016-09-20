#pragma once
#ifndef ANYTIMEEGOALOMODEL_H
#define ANYTIMEEGOALOMODEL_H

#include "stdafx.h"
#include "ModelBasedEgoAlo.h"

//The goal for anytime egoAlo is to create several different scales of ego learners.
//there should be learners that can learn in short term, and learners  that will learn in the longer term. 
//The Short Term will be less accurate, while the long term will take more to train. 
//The states given to the learner must be at their full definition desired for long term, the short term will reduce the states from there
//This probably only works with MDP's that can be actually reduced linearly such as a continuous system like angles.
class AnyTimeEgoAloModel :
	public ModelBasedEgoAlo
{

private:

	typedef vector<double>  stateType;
	typedef vector<double> actionType;
	typedef unordered_map<stateType, map<actionType, map<stateType, pair<double, double>>>>  SASTable;
	typedef unordered_map<stateType, map<actionType, double>> QTableType;

public:
	AnyTimeEgoAloModel() : egoSize(NULL) {};
	AnyTimeEgoAloModel(const vector<vector<double>>& AvailActions, const vector<double>& StartState, const int EgoSize);
	AnyTimeEgoAloModel(const vector<vector<double>>& AvailActions, const vector<double>& StartState, const int EgoSize, double DefQ, double gam, int MaxUps);
	virtual ~AnyTimeEgoAloModel();

	double Update(const StateTransition & transition);
	double SetQValues(const std::vector<double>& state, const std::vector<vector<double>>& actions, const int Qval);

private:
	void updatePredictionModels(const stateType& oldEgo, const actionType& act, const stateType& newAlo, const stateType& oldAlo, const double rew);
	stateType getDefinitionState(int scaler,const stateType& ego);

	//Serialization
	friend class boost::serialization::access;
	template<class Archive>
	inline void serialize(Archive & ar, const unsigned int version);

	//****MEMBERS****//
	ModelBasedLearning aloLearner, rewardPredictionModel;
	//This is the Ego part of the learner, each model uses .value to return its prediction on what change in feature is
	vector<ModelBasedLearning> LowDefAloFeaturePredictionModels, MedDefAloFeaturePredictionModels, HighDefAloFeaturePredictionModels;
	vector<vector<double>> availableActions;	   //Kept for re-setting the model-Based(alocentric) learner
	vector<double> startState;
	vector<unordered_map<stateType, int>> visitedStates; //Keep track #-times an action has led to an alo-state.
	//Keep Track #-times an action has been seen at an ego state
	map<actionType, unordered_map<stateType, int>> lowVisitedEgoStates,medVisitedEgoStates,highVisitedEgoStates;
	PerformanceStats perfStats;

	int updateTerminationStepCount = 1;
	int egoSize;
	double gamma = 0.9, defQ = 10, maxUps = 120;
	int steps = 0, minStepsRequired = 15;//How many steps this class has seen taken, (number of times that update has been called)

	//State factoring variables, these will essentially take the values and reduce, rounding to the nearest value
	int lowDef = 10, medDef = 5, highDef = 1;


};

#endif


template<class Archive>
inline void AnyTimeEgoAloModel::serialize(Archive & ar, const unsigned int version)//Does not provide the start state, parameters (except egoSize)
{
	ar & boost::serialization::base_object<ModelBasedEgoAlo>(*this);
	ar & availableActions;
	ar & startState;
	ar & rewardPredictionModel;
	ar & LowDefAloFeaturePredictionModels;
	ar & MedDefAloFeaturePredictionModels;
	ar & HighDefAloFeaturePredictionModels;
	ar & aloLearner;
	ar & lowVisitedEgoStates;
	ar & medVisitedEgoStates;
	ar & highVisitedEgoStates;
	ar & visitedStates;
	ar & egoSize;
	ar & gamma;
	ar & defQ;
	ar & maxUps;
	ar & steps;
	ar & minStepsRequired;
}

BOOST_CLASS_EXPORT_KEY(AnyTimeEgoAloModel);