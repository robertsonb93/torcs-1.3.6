#pragma once
#ifndef _SOFTMAXPOLICY_H
#define _SOFTMAXPOLICY_H

#include "stdafx.h"
#include "PolicyBase.h"

	class  SoftmaxPolicy : public PolicyBase
	{
	public:
		SoftmaxPolicy();
		SoftmaxPolicy(double diffDefault);

		~SoftmaxPolicy();

		std::vector<double> selectAction(std::vector<std::vector<double>>& availableActions, std::vector<double>& values, std::vector<double>& params);
		bool setDefaultT(double newT);

	private:
		//Members//
		double defaultT = 1;
	};

#endif

