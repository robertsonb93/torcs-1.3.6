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
	double Value(const vector<double>& state, const vector<double>& action);
	vector<double> Value(const vector<double>& state, const vector<vector<double>>& actions);
	void ResetAllocentric();



	//Get/Set the Performance Stats
	PerformanceStats& GetStats();
	void SetStats(PerformanceStats& PS);
	void ResetStats();

private:

	void updatePredictionModels(const stateType& oldEgo,const actionType& act,const stateType& newAlo,const stateType& oldAlo,const double rew);

	//****MEMBERS****//
	ModelBasedLearning aloLearner,rewardPredictionModel;
	vector<ModelBasedLearning> aloFeaturePredictionModels; //This is the Ego part of the learner, each model uses .value to return its prediction on what change in feature is

	//Kept for re-setting the model-Based(alocentric) learner
	vector<vector<double>> availableActions;
	vector<double> startState; 
	
	vector<unordered_map<stateType, int>> visitedStates; //Keep track #-times an action has led to an alo-state. 
	map<actionType,unordered_map<stateType, int>> visitedEgoStates;//Keep Track #-times an action has been seen at an ego state
	PerformanceStats perfStats;

	int updateTerminationStepCount = 1;
	 int egoSize;
	double gamma = 0.9, defQ = 10, maxUps = 120;
	int steps = 0,minStepsRequired = 10;//How many steps this class has seen taken, (number of times that update has been called)

};
#endif

//template<class Archive>
//inline void ModelBasedEgoAlo::serialize(Archive & ar, const unsigned int version)//Does not provide the start state, parameters (except egoSize)
////{
//	cerr <<endl<< "Entered ModelBasedEgoAlo Serialize" << endl;
//
//	//ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ActionValue);
//	//ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModelBasedBase);
//	//ar & boost::serialization::base_object<ModelBasedBase>;
//	ar & boost::serialization::make_nvp("Alo",aloLearner);
//	ar & availableActions;
//	ar & rewardPredictionModel;
//	ar & aloFeaturePredictionModels;
//	ar & visitedStates;
//	ar & visitedEgoStates;
//	ar & egoSize;
//	cerr << endl << "finished ModelBasedEgoAlo Serialize" << endl;
//}

//BOOST_CLASS_EXPORT_KEY(ModelBasedEgoAlo);