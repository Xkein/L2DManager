#pragma once

#include "l2d\l2dManager.h"
#include "l2d\l2dModel.h"
#include "l2d\l2dPal.h"
enum class LostStep
{
	LostStep_None,
	LostStep_Lost,
	LostStep_ReCreate,
};


void CommitFunction();

bool Delay();
