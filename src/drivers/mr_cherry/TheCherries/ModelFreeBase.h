#pragma once
#ifndef _MODELFREEBASE_H
#define _MODELFREEBASE_H

#include "stdafx.h"
#include "ActionValue.h"


extern "C"
{
	//Used for enforcing specific behaviours from model free classes
	class  ModelFreeBase abstract : public ActionValue
	{
	public:

		ModelFreeBase()
		{
		}

		virtual ~ModelFreeBase() = 0 {};
	};
}
#endif
