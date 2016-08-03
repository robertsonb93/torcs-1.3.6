#pragma once
#ifndef MULTI_AGENT_H
#define MULTI_AGENT_H

#include "stdafx.h"
#include "AgentBase.h"


class AgentMulti :
	public AgentBase
{
public:
	AgentMulti();
	~AgentMulti();

	std::vector<double> SelectAction();
	std::vector<double> SelectAction(vector<double>& params);//For use with policies that have parameters
	void LogEvent(StateTransition transition);
	PerformanceStats GetStats();
	void SetStats(PerformanceStats& PS);
	void SetState(vector<double>& inputState);
	vector<double> GetState();
};

#endif