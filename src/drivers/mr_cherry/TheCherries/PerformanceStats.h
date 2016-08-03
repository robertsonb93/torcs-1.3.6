#pragma once
#ifndef _PERFORMANCESTATS_H
#define _PERFORMANCESTATS_H

#include "stdafx.h"
//#include <vector>
#if defined _EXPORT
#define  _declspec(export)
#else
#define  _declspec(import)
#endif


	using namespace std;

	class PerformanceStats
	{
	public:
		 PerformanceStats();
		 PerformanceStats(const double cmRwrd, const int modAcc, const int modUp);
		 ~PerformanceStats();

		 void TallyStepsToGoal(bool goalReached);

		 double GetCumulativeReward();
		 int GetModelAccesses();
			 int GetModelUpdates();
		 vector<double> GetStepsToGoal(int indexStart = -1,int indexEnd = -1);

		 void SetCumulativeReward(const double n);
		 void SetModelAccesses(const int n);
		 void SetModelUpdates(const int n);


		//Members//
	private:

		vector<double> stepsToGoal;

		double cumulativeReward;
		int modelAccesses;
		int modelUpdates;


	};

#endif
