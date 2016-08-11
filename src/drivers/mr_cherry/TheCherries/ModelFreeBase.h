#pragma once
#ifndef _MODELFREEBASE_H
#define _MODELFREEBASE_H

#include "stdafx.h"
#include "ActionValue.h"



	//Used for enforcing specific behaviours from model free classes
	class  ModelFreeBase abstract : public ActionValue
	{
	//private:
	//	//Serialization
	//	friend class boost::serialization::access;
	//	template<class Archive>
	//	void serialize(Archive & ar, const unsigned int version) { ar & boost::serialization::base_object<ActionValue>(*this); };

	//
	public:

		ModelFreeBase()
		{
		}

		virtual ~ModelFreeBase() = 0 {};
	};

#endif
	//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ModelFreeBase);
