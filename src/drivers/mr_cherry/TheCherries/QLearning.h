#pragma once
#ifndef _QLEARNING_H
#define _QLEARNING_H

#include "stdafx.h"
#include "ModelFreeBase.h"

//Class for the QLearning, derived class from ModelFreeBase which is Derived from ActionValue
//
	using namespace std;
	class QLearning : public ModelFreeBase
	{
	public:
		 QLearning(const vector<vector<double>> AA);
		 QLearning(const vector<vector<double>> AA, const double alph, const double gam, const double defQ);
		 ~QLearning();

		 double Value(const vector<double>& state, const vector<double>& action);
		 vector<double> Value(const vector<double>& state, const vector<vector<double>>& actions);
		 double Update(const StateTransition & transition);
		 PerformanceStats& GetStats();
		 void SetStats(PerformanceStats& PS);		

	private:

		//Serialization
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version);

		//Members//
		double alpha;
		double gamma;
		double defaultQ;
		//First element is the statevector, this returns a map that given an action returns the q-value
		unordered_map<vector<double>, map<vector<double>, double>> table;
		vector<vector<double>> availableActions;
		PerformanceStats stats;
		map<vector<double>, double> defaultMap;



	};

	template<class Archive>
	inline void QLearning::serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ModelFreeBase>(*this);
		ar & table;
		std::cout << QTable.size() << std::endl;
		ar & defaultMap;
		ar & availableActions;
	}

	BOOST_CLASS_EXPORT_KEY(QLearning);

#endif
