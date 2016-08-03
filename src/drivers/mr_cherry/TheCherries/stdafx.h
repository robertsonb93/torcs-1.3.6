#pragma once
#ifndef _STDAFX_H
#define _STDAFX_H
/*
	WHENEVER YOU INCLUDE stdafx.h, max sure it is the FIRST include "LIKELY first command after include guards"
  Else you must face the wrath of your program compiling like poopoo
  */

//stdafx is a known as a pre-comnpiled header,
//It lets the compiler only have to compile each 
//header file once in the stdafx
//Then after it can use the saved compilation for stdafx and include into everything else
//This is verses having a class like vector being recompiled in multiple different classes.

//Do not use Header files that are defined in the project or considered unfinished
// as this would cause a change to a single file 
//to mean the entire project needs recompiling

#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <time.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <list>


//From the Boost Library For serializing different types of STL objects
//#include "vector.hpp" //Serialize a vector
//#include "map.hpp"
//#include "hash_map.hpp"
//
////After a class is serialized, it must be saved as an archive
//#include "text_iarchive.hpp";
//#include "text_oarchive.hpp";

//You can uncomment any these if they are considered stable/final

////Auxiliary classes
#include "PerformanceStats.h"
#include "StateTransition.h"
#include "HashVecDoubles.h"
//
////Agent classes
#include "AgentBase.h"
//#include "AgentSingle.h"
//
////World Classes
//#include "GridWorldBase.h"
#include "WorldBase.h"
//#include "TraditionalMaze.h"
//
////ActionValue Classes
//#include "ActionValue.h"
//#include "ModelBasedBase.h"
//#include "ModelFreeBase.h"
//#include "QLearning.h"
//
////Policy Classes
#include "PolicyBase.h"
#include "EpsilonGreedy.h"
//#include "SoftmaxPolicy.h"
#include "OptimalPolicy.h"

#endif