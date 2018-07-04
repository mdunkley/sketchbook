#pragma once

#include <stdio.h>
#include "cinder/audio/audio.h"

#include "cinder/Rand.h"
#include "CommonUtils.h"



using namespace cinder::audio;

class RMSNode : public ci::audio::Node
{
public:
	RMSNode();
	~RMSNode();
private:
	double mMaxAverage = 48000;
	double mAccum = 0.0;
	BufferDynamicRef mBuffer;
};

