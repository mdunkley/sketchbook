#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#pragma once

class Sequence
{
public:
	Sequence();
	~Sequence();

	long mClockCount = 0;
	long mTriggerCount = 0;

	std::vector<int> mTriggerLane;
	std::vector< std::vector<float> > mModulationLanes;
};

