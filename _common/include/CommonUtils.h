#pragma once

#include <list>
#include <array>
#include <stdio.h>
#include "cinder/CinderMath.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"

using namespace cinder::log;

int wrap(int kX, int const kLowerBound, int const kUpperBound);

int closest(int value, const std::list<int> & vec);

int quantize(int n, std::list<int>& s);

namespace cinder {
	namespace audio {

		inline float interpLinear(const float *array, size_t arraySize, float readPos)
		{
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1) % arraySize;
			float val1 = array[index1];
			float val2 = array[index2];
			float frac = readPos - (float)index1;

			return val2 + frac * (val2 - val1);
		}

		inline float interpLinearByChannel(const float *array, size_t channel, size_t channelSize, float readPos)
		{
			size_t channelOffset = channelSize * channel;
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1);
			float val1 = array[channelOffset + (index1%channelSize)];
			float val2 = array[channelOffset + (index2%channelSize)];
			float frac = readPos - (float)index1;

			return val2 + frac * (val2 - val1);
		}

		inline float interpCosine(const float *array, size_t arraySize, float readPos)
		{
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1) % arraySize;
			float val1 = array[index1];
			float val2 = array[index2];
			float frac = readPos - (float)index1;

			float mu2 = (1 - cos(frac*M_PI)) / 2;
			return(val1*(1 - mu2) + val2 * mu2);
		}

		inline float interpCosineByChannel(const float *array, size_t channel, size_t channelSize, float readPos)
		{
			size_t channelOffset = channelSize * channel;
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1);
			float val1 = array[channelOffset + (index1%channelSize)];
			float val2 = array[channelOffset + (index2%channelSize)];
			float frac = readPos - (float)index1;

			float mu2 = (1 - cos(frac*M_PI)) / 2;
			return(val1*(1 - mu2) + val2 * mu2);
		}
	}
}

namespace AudioOp {

	class Delay {

	public:

		//float operator()(float fl) { ; }
		float process(float invalue, size_t delaysamples) {
			int totaldelay = std::min(delaysamples, mMaxDelayLength);
			mDelay[mWriteHead] = invalue;
			mReadHead = mWriteHead - delaysamples;
			if (mReadHead < 0) mReadHead += mMaxDelayLength;
			if ((++mWriteHead) >= mMaxDelayLength) mWriteHead = 0;
			return mDelay[mReadHead];
		}

	private:

		static const size_t mMaxDelayLength = 65536;
		long mReadHead = 0;
		long mWriteHead = 0;
		std::array<float, mMaxDelayLength> mDelay;

	};

	class History {
	public:
		float process(float value) {
			float outvalue = mPreviousValue;
			mPreviousValue = value;
			return mPreviousValue;
		}
	private:
		float mPreviousValue = 0;
	};

	class Delta {
	public:
		float process(float value) {
			float outvalue = value - mPreviousValue;
			mPreviousValue = value;
			return outvalue;
		}
	private:
		float mPreviousValue = 0;
	};

	class Change {
	public:
		float process(float value) {
			float outvalue = 0;
			if (value > mPreviousValue) outvalue = 1;
			else if (value < mPreviousValue) outvalue = -1;
			mPreviousValue = value;
			return outvalue;
		}
	private:
		float mPreviousValue = 0;
	};

	class RisingEdgeDetector {
	public:

		enum class State { wait, arm, active };

		float process(float value, float gatelength = 0) {

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
						//CI_LOG_I(ci::audio::master()->getNumProcessedSeconds());
						mTimer = 1;
						State::wait;
					}
				} else {
					float newTimer = ci::audio::master()->getNumProcessedSeconds();
					//CI_LOG_I(newTimer - mPrevTimer << " " << mPrevTimer << " ";);
					mPrevTimer = newTimer;
					outvalue = 1;
					mState = State::wait;
				}
			}

			mPrevValue = value;
			return outvalue;
		}

	private:

		State mState = State::wait;
		double mEnvInc;
		double mTimer;
		float mPrevValue;
		double mPrevTimer = 0;
	};
}