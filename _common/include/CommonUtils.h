#pragma once

#include <list>
#include <array>
#include <stdio.h>
#include "cinder/CinderMath.h"

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

	class TriggerDetect {
	public:
		TriggerDetect() { mPrevValue.fill(0); }

		float process(float value, int ch = 0, float thresh = .1f) { 
			float tval = value > thresh;
			float outValue = ci::clamp(mPrevValue[ch] - tval,0.0f,1.0f);
			mPrevValue[ch] = tval;
			return tval;
		}

	private:
		std::array<float,32> mPrevValue;

	};
}