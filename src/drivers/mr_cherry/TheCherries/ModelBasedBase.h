#pragma once
#ifndef _MODELBASEDBASE_H
#define _MODELFBASEDBASE_H

#include "stdafx.h"
#include "ActionValue.h"




	//Can Design an enforce any needed ModelBased bahaviour through this abstract class
	class  ModelBasedBase abstract : public ActionValue
	{
	public:
		ModelBasedBase() {};
		ModelBasedBase(const std::vector<std::vector<double>> availableActions, const std::vector<double> StartState) {};
		virtual ~ModelBasedBase() = 0 {};
	private:
	//	//Serialization
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) 
		{ cout << endl << "Entered ModelBasedBase Serialize" << endl; 
		ar & boost::serialization::base_object<ActionValue>(*this); 
		cout << endl << "Entered ModelBasedBase Serialize" << endl;
		};
	};

	BOOST_SERIALIZATION_ASSUME_ABSTRACT(ModelBasedBase);
	//BOOST_CLASS_EXPORT_KEY(ModelBasedBase);
#endif
	