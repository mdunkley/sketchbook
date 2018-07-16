#include "Circuits.h"
#include <list>
#include "cinder/CinderMath.h"
#include "CommonUtils.h"


namespace Circuits {

	float SampleAndHold::process(float invalue, float gatesig) {
		if (gatesig > 0) mValue = invalue;
		return mValue;
	}


	// DELAY

	Delay::Delay() 
	{
		setDelaySize(mDelayMax);
	}

	Delay::~Delay() {
	}
	
	void Delay::setDelaySize(size_t samples) {

		size_t value = std::max(samples, (size_t)0);

		if (value > mDelaySize) {

			mDelayLine.resize(value);
			std::fill(mDelayLine.begin(), mDelayLine.end(), 0);

		}
		mDelaySize = value;

	}

	float Delay::process(float invalue, size_t samples) {

		setDelaySize(samples);
		return process(invalue);

	}

	float Delay::process(float invalue) {

		//CI_LOG_I(invalue << " " << mReadHead << " " << mDelayLine.size() << " " << mDelaySize);
		if (mDelayLine.size() > 0) {
			mDelayLine.at(mWriteHead) = invalue;
			mReadHead = wrap(mWriteHead - std::min(mDelaySize, mDelayMax - 1), 0, mDelayMax);
			mWriteHead++;
			if (mWriteHead >= mDelayMax) mWriteHead = 0;

			//CI_LOG_I(mReadHead<<" "<<mDelayMax<<" "<<mDelayLine.size());
			return mDelayLine.at(mReadHead);
		}
		else {
			return invalue;
		}

	}

	float History::process(float value) {

		float outvalue = mPreviousValue;
		mPreviousValue = value;
		return mPreviousValue;

	}

	float RisingEdgeTrigger::process(float value, float gatelength) {

		float outvalue = 0;

		if (mState == State::wait) {
			if (value < mPrevValue) mState = State::arm;
		}
		else if (mState == State::arm) {
			if (value > mPrevValue) {
				mState = State::active;
				mEnvInc = 1 / gatelength;
				mTimer = 0;
			}
		}
		else if (mState == State::active) {
			if (gatelength > 0) {
				outvalue = 1;
				mTimer += mEnvInc;
				if (mTimer >= 1) {
					mTimer = 1;
					State::wait;
				}
			}
			else {
				float newTimer = ci::audio::master()->getNumProcessedSeconds();
				mPrevTimer = newTimer;
				outvalue = 1;
				mState = State::wait;
			}
		}

		mPrevValue = value;
		return outvalue;
	}
}