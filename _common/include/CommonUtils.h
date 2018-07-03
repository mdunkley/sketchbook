#pragma once

#include <list>
#include "cinder/CinderMath.h"



inline int wrap(int kX, int const kLowerBound, int const kUpperBound)
{
	int range_size = kUpperBound - kLowerBound + 1;

	if (kX < kLowerBound)
		kX += range_size * ((kLowerBound - kX) / range_size + 1);

	return kLowerBound + (kX - kLowerBound) % range_size;
}

struct ClosestCmp {
	bool operator()(const int & x, const int & y) { return x > y; }
};

inline int closest(int value, const std::list<int> & vec)
{

	std::list<int>::const_reverse_iterator cri =
		std::lower_bound(vec.rbegin(), vec.rend(), value, ClosestCmp());
	if (cri != vec.rend()) {

		return *cri;

	}
	return -1;
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inline int quantize(int n, std::list<int>& s) {

	if (s.size() > 0) {
		int offset = n + 288;
		int octave = (offset / 12) * 12;
		s.push_front(*(--s.end()) - 12);
		s.push_back(*(s.begin()) + 12);
		float output = closest(abs(offset - octave), s) + octave;
		return output - 288;
	}
	else return n;

}

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