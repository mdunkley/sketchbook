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
		size_t delayLineSize = mDelayLine.size();
		//CI_LOG_I(invalue << " " << mReadHead << " " << mDelayLine.size() << " " << mDelaySize);
		if (delayLineSize > 0) {

			mWriteHead = wrap( mWriteHead, 0, delayLineSize-1 );
			mDelayLine[ mWriteHead ] = invalue;
			mReadHead = wrap( mWriteHead - std::min( mDelaySize, delayLineSize - 1), 0, delayLineSize - 1);
			mWriteHead++;

			return mDelayLine.at( mReadHead );
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

	float AREnvelope::operator()(float triggerSig)
	{
		int trigger = triggerSig > .025;
		if (mState == State::wait) {
			mValue = 0;
			if (trigger) {
				mInc = 1.0 / mAttackLength;
				mState = State::attack;
			}
		}
		else if (mState == State::attack) {
			mValue += mInc;
			if (mValue >= 1) {
				mValue = 1;
				mInc = -1.0 / mDecayLength;
				mState = State::decay;
			}
		}
		else if (mState == State::decay) {
			mValue += mInc;
			if (mValue <= 0) {
				mValue = 0;
				mInc = 0;
				mState = State::wait;
			}
		}
		return mValue;
	}


	float AREnvelope::operator()(float triggerSig, float attack, float decay)
	{
		mAttackLength = attack;
		mDecayLength = decay;
		return (*this)(triggerSig);
	}
}
