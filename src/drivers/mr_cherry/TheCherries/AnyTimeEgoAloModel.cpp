#include "stdafx.h"
#include "AnyTimeEgoAloModel.h"
BOOST_CLASS_EXPORT_IMPLEMENT(AnyTimeEgoAloModel);

AnyTimeEgoAloModel::AnyTimeEgoAloModel(const vector<vector<double>>& AvailActions, const vector<double>& StartState, const int EgoSize)
	:egoSize(EgoSize), availableActions(AvailActions), startState(StartState), aloLearner(AvailActions, StartState),
	rewardPredictionModel(ModelBasedLearning(AvailActions, StartState, 0, 0, 120))
{
	for (unsigned int i = egoSize; i < startState.size(); i++)
	{
		HighDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
		MedDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
		LowDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
	}
	visitedStates.resize(AvailActions.size());
}

AnyTimeEgoAloModel::AnyTimeEgoAloModel(const vector<vector<double>>& AvailActions, const vector<double>& StartState, const int EgoSize, double DefQ, double gam, int MaxUps)
	: egoSize(EgoSize), availableActions(AvailActions), startState(StartState), aloLearner(AvailActions, StartState, DefQ, gam, MaxUps),
	rewardPredictionModel(ModelBasedLearning(AvailActions, StartState, DefQ, 0, MaxUps)), gamma(gam), defQ(DefQ), maxUps(MaxUps)
{
	for (unsigned int i = egoSize; i < startState.size(); i++)
	{
		HighDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
		MedDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
		LowDefAloFeaturePredictionModels.push_back(ModelBasedLearning(AvailActions, StartState, 0, 0, 120));
	}
	visitedStates.resize(AvailActions.size());
}

AnyTimeEgoAloModel::~AnyTimeEgoAloModel()
{
}

double AnyTimeEgoAloModel::Update(const StateTransition & transition)
{
	steps++;
	auto oldState = transition.getOldState();
	auto newState = transition.getNewState();
	auto action = transition.getAction();

	const auto egoOldBegin = oldState.begin();
	const auto egoNewBegin = newState.begin();
	const auto aloOldBegin = egoOldBegin + egoSize;
	const auto aloNewBegin = egoNewBegin + egoSize;

	//Firstly we will get the respective ego and alo state from the transition which contains the state(ego & alo)
	//Will use iterators to use a copy constructor for each of the different states.
	stateType egoOldState(egoOldBegin, aloOldBegin);//The copy goes up to but not including the last iterator. 
	stateType egoNewState(egoNewBegin, aloNewBegin);
	stateType alloOldState(aloOldBegin, oldState.end());
	stateType alloNewState(aloNewBegin, newState.end());

	updatePredictionModels(egoOldState, action, alloOldState, alloNewState, transition.getReward());

	auto lowDefActIter = lowVisitedEgoStates.emplace(action, unordered_map<stateType, int>()).first;
	auto medDefActIter = medVisitedEgoStates.emplace(action, unordered_map<stateType, int>()).first;
	auto highDefActIter = highVisitedEgoStates.emplace(action, unordered_map<stateType, int>()).first;

	lowDefActIter->second.emplace(getDefinitionState(lowDef, egoOldState), 0).first->second++;
	medDefActIter->second.emplace(getDefinitionState(medDef, egoOldState), 0).first->second++;
	highDefActIter->second.emplace(getDefinitionState(highDef, egoOldState), 0).first->second++;

	aloLearner.Update(StateTransition(alloOldState, alloNewState, action, transition.getReward()));

	if (egoSize)//If there is an ego to be used.
	{
		for (unsigned int i = 0; i < availableActions.size(); i++)
		{
			auto visitedStateIter = visitedStates[i].emplace(alloNewState, 0).first;
			visitedStateIter->second++; //Regardless of if we have seen before or not, we will increment, so we mark 1, or if not emplaced then whatever was in place.

			if (steps >= minStepsRequired && visitedStateIter->second <= updateTerminationStepCount)
			{
				const auto& currAct = availableActions[i];
				const auto& lowseenAct = lowVisitedEgoStates.find(currAct); 
				const auto& medseenAct = medVisitedEgoStates.find(currAct); 
				const auto& highseenAct = highVisitedEgoStates.find(currAct);

				stateType highEgoNewState(getDefinitionState(highDef, egoNewState));
				stateType medEgoNewState(getDefinitionState(medDef, egoNewState));
				stateType lowEgoNewState(getDefinitionState(lowDef, egoNewState));

				if (highseenAct != highVisitedEgoStates.end() && (highseenAct->second.find(highEgoNewState) != highseenAct->second.end()))
				{
					stateType predictedAlo(alloNewState);

					for (unsigned int j = 0; j < HighDefAloFeaturePredictionModels.size(); j++)
						predictedAlo[j] += HighDefAloFeaturePredictionModels[j].Value(highEgoNewState, currAct);//Get all the predicted changes for each alocentric feature	

					PerformanceStats tempStats = aloLearner.GetStats();
					aloLearner.Update(StateTransition(alloNewState, predictedAlo, currAct, double(rewardPredictionModel.Value(highEgoNewState, currAct))));
					aloLearner.SetStats((tempStats));
				}
				else if (medseenAct != medVisitedEgoStates.end() && (medseenAct->second.find(medEgoNewState) != medseenAct->second.end()))
				{
					stateType predictedAlo(alloNewState);

					for (unsigned int j = 0; j < MedDefAloFeaturePredictionModels.size(); j++)
						predictedAlo[j] += MedDefAloFeaturePredictionModels[j].Value(medEgoNewState, currAct);//Get all the predicted changes for each alocentric feature	

					PerformanceStats tempStats = aloLearner.GetStats();
					aloLearner.Update(StateTransition(alloNewState, predictedAlo, currAct, double(rewardPredictionModel.Value(medEgoNewState, currAct))));
					aloLearner.SetStats((tempStats));
				}

				else if (lowseenAct != lowVisitedEgoStates.end() && (lowseenAct->second.find(lowEgoNewState) != lowseenAct->second.end()))
				{
					stateType predictedAlo(alloNewState);

					for (unsigned int j = 0; j < LowDefAloFeaturePredictionModels.size(); j++)
						predictedAlo[j] += LowDefAloFeaturePredictionModels[j].Value(lowEgoNewState, currAct);//Get all the predicted changes for each alocentric feature	

					PerformanceStats tempStats = aloLearner.GetStats();
					aloLearner.Update(StateTransition(alloNewState, predictedAlo, currAct, double(rewardPredictionModel.Value(lowEgoNewState, currAct))));
					aloLearner.SetStats((tempStats));
				}


			}
		}

	}

	return 0.0;
}

double AnyTimeEgoAloModel::SetQValues(const std::vector<double>& state, const std::vector<vector<double>>& actions, const int Qval)
{
	vector<double> egoState(state.begin(), state.begin() + egoSize), alostate(state.begin() + egoSize, state.end());
	stateType lowEgoState(getDefinitionState(lowDef, egoState));
	stateType medEgoState(getDefinitionState(medDef, egoState));
	stateType hiEgoState(getDefinitionState(highDef, egoState));

	aloLearner.SetQValues(alostate, actions, Qval);

	for (int i = 0; i < LowDefAloFeaturePredictionModels.size(); i++)
	{
		LowDefAloFeaturePredictionModels[i].SetQValues(lowEgoState, actions, Qval);
		MedDefAloFeaturePredictionModels[i].SetQValues(medEgoState, actions, Qval);
		HighDefAloFeaturePredictionModels[i].SetQValues(hiEgoState, actions, Qval);
	}

	rewardPredictionModel.SetQValues(lowEgoState, actions, Qval);
	rewardPredictionModel.SetQValues(medEgoState, actions, Qval);
	rewardPredictionModel.SetQValues(hiEgoState, actions, Qval);

	return 0;
}

inline void AnyTimeEgoAloModel::updatePredictionModels(const stateType & oldEgo, const actionType & act, const stateType & newAlo, const stateType & oldAlo, const double rew)
{
	if (egoSize)
	{
		stateType lowEgoState(getDefinitionState(lowDef, oldEgo));
		stateType medEgoState(getDefinitionState(medDef, oldEgo));
		stateType hiEgoState(getDefinitionState(highDef, oldEgo));
		stateType dummyNewState = { numeric_limits<double>::lowest() };
		for (unsigned int i = 0; i < LowDefAloFeaturePredictionModels.size(); i++)
		{
			double aloChange = oldAlo[i] - newAlo[i];
			//It doesn't need to learn of an S' as there is no continuity on the ego side of things, so we give a dummy state. 
			//The reward is what it is predicting, but reward is actually the feature change.
			LowDefAloFeaturePredictionModels[i].Update(StateTransition(lowEgoState, dummyNewState, act, (aloChange)));
			MedDefAloFeaturePredictionModels[i].Update(StateTransition(medEgoState, dummyNewState, act, (aloChange)));
			HighDefAloFeaturePredictionModels[i].Update(StateTransition(hiEgoState, dummyNewState, act, (aloChange)));
		}
		rewardPredictionModel.Update(StateTransition(lowEgoState, dummyNewState, act, rew));
		rewardPredictionModel.Update(StateTransition(medEgoState, dummyNewState, act, rew));
		rewardPredictionModel.Update(StateTransition(hiEgoState, dummyNewState, act, rew));
	}


}


//100 is a magic number, it has not been developed far enough to decide what a good amount of precision for the values are. So
//For what I know of my current state space, 100 should work.
inline AnyTimeEgoAloModel::stateType AnyTimeEgoAloModel::getDefinitionState(int scaler, const stateType & ego)
{
	stateType ret;
	for (int i = 0; i < ego.size(); i++)
	{
		int val = ego[i] * 100;
		val /= scaler;
		val *= scaler;
		double newVal = val / 100.00;
		ret.push_back(newVal);
	}
	return ret;
}
