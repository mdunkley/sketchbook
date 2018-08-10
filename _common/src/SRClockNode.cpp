#include <cmath>
#include "SRClockNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"
#include "cinder/Rand.h"

using namespace cinder::log;

SRClockNode::~SRClockNode()
{
}

void SRClockNode::initialize()
{
}

void SRClockNode::tick() {

	mTimer = 0;

	mNextTick = abs(mRate * mClockDivisions
		+ (mRateJitter*cinder::Rand::randFloat(-1, 1)))
		* ci::audio::master()->getSampleRate();

	mDuty = abs(mDutyCycle + mDutyCycleJitter * cinder::Rand::randFloat(-1, 1));

	mTickInc = 1.0 / mNextTick;

}

void SRClockNode::process(ci::audio::Buffer *buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	size_t numFrames = frameRange.second - frameRange.first;
	float *data = buffer->getData();
	const float *syncData = nullptr;

	if (mSync.eval()) syncData = mSync.getValueArray();

	int readCount = 0;

	while (readCount < numFrames) {
	
		if (syncData) {

			float newValue = syncData[readCount];
			float offset = newValue - mOldSyncValue;
			if (offset < 0) mWaitingForRisingEdge = true;

			if (mWaitingForRisingEdge && offset > 0) {

				mWaitingForRisingEdge = false;
				auto oldSyncTime = mSyncTime;
				mSyncTime = ci::audio::master()->getNumProcessedSeconds();
				mRate = mSyncTime - oldSyncTime;

				mSyncCount++;
				if (mSyncCount >= mClockDivisions) {
					tick();
					mSyncCount = 0;
				}

			}
			mOldSyncValue = newValue;
		}
		else if (mTimer >= mNextTick) tick();	
		

		mTimer++;

		float ramp = float(mTimer) / mNextTick;
		
		for (int ch = 0; ch < numChannels; ch++) {

			size_t lookup = ch * numFrames + readCount;

			if (mMode == OutputMode::gate) {data[lookup] = ramp < mDuty;} 
			else if (mMode == OutputMode::ramp) {data[lookup] = ramp;}
		}

		readCount++;

	}
}