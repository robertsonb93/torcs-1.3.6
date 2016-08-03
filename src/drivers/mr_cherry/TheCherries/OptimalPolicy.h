#pragma once
#ifndef _OPTIMALPOLICY_H
#define _OPTIMALPOLICY_H

#include "stdafx.h"
#include "PolicyBase.h"

//Want to see an abnormally complicated class that performs a single function, then check this ballin class out, its just a comparitor but it has a constructor

	//Simple policy that selects the highest value action given a state
	using namespace std;
	class  OptimalPolicy : public PolicyBase
	{
	public:
		OptimalPolicy();
		~OptimalPolicy();

		vector<double> selectAction(std::vector<std::vector<double>>& availableActions,std::vector<double>& values,std::vector<double>& params);
	

	private:
		vector<double>::iterator it, newIt,end;
	};
#endif