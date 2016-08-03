#pragma once
#ifndef _MODELBASEDBASE_H
#define _MODELFBASEDBASE_H

#include "stdafx.h"
#include "ActionValue.h"


extern "C"
{

	//Can Design an enforce any needed ModelBased bahaviour through this abstract class
	class  ModelBasedBase abstract : public ActionValue
	{
	public:
		ModelBasedBase() {};
		ModelBasedBase(const std::vector<std::vector<double>> availableActions, const std::vector<double> StartState) {};
		virtual ~ModelBasedBase() = 0 {};


	};
}

#endif