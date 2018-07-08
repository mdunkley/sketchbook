#include <cmath>
#include "ClockNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"
#include "cinder/Rand.h"

using namespace cinder::log;

ClockNode::ClockNode(const Format & format) :
	Node(format)
{
}

ClockNode::~ClockNode()
{
}

void ClockNode::initialize()
{

}



void ClockNode::tick() {

	mTimer = 0;
	mNextTick = abs(mRate
		+ (mRateJitter*cinder::Rand::randFloat(-1, 1)))
		* ci::audio::master()->getSampleRate();
	mDuty = abs(mDutyCycle + mDutyCycleJitter * cinder::Rand::randFloat(-1, 1));
	mTickInc = 1.0 / mNextTick;

}

void ClockNode::process(ci::audio::Buffer *buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	size_t numFrames = frameRange.second - frameRange.first;
	float *data = buffer->getData();

	int readCount = 0;

	while (readCount < numFrames) {

		if (mTimer >= mNextTick) tick();
		mTimer++;

		float ramp = float(mTimer) / mNextTick;
		for (int ch = 0; ch < numChannels; ch++) {
			size_t lookup = ch * numFrames + readCount;
			data[lookup] = ramp<mDuty;
		}

		readCount++;

	}
}