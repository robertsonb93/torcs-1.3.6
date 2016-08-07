#pragma once
#ifndef _ACTIONVALUEBASE_H
#define _ACTIONVALUEBASE_H


#include "stdafx.h"//#include <vector>


//Provides the abstract base class for the ActionValue's, the ActionValues are the individual learning Algorithms


//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ActionValue);
	class  ActionValue abstract
	{
	public:

		ActionValue()
		{
		}

		virtual ~ActionValue() = 0 {};


		 virtual double Value(const std::vector<double>& state, const std::vector<double>& action) = 0;
		 virtual vector<double> Value(const std::vector<double>& state, const std::vector<vector<double>>& actions) = 0;
		 virtual double Update(const StateTransition & transition) = 0;
		 virtual PerformanceStats& GetStats() = 0;
		 virtual void SetStats(PerformanceStats& PS) = 0;
		 
	private:

		////Serialization
		friend class boost::serialization::access;
		//template<class Archive>
	 //void serialize(Archive & ar, const unsigned int version) {
		// { cout << endl << "Entered ActionValue Serialize" << endl;
		// };
	 

		//Members//
		//std::vector<double> availableActions //Every learning will need a list of availactions


	};
	BOOST_SERIALIZATION_ASSUME_ABSTRACT(ActionValue);
//	BOOST_CLASS_EXPORT_KEY(ActionValue);
#endif
