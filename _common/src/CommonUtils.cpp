#include "CommonUtils.h"
#include <list>
#include "cinder/CinderMath.h"

int wrap(int kX, int const kLowerBound, int const kUpperBound)
{
	int range_size = kUpperBound - kLowerBound + 1;

	if (kX < kLowerBound)
		kX += range_size * ((kLowerBound - kX) / range_size + 1);

	return kLowerBound + (kX - kLowerBound) % range_size;
}


struct ClosestCmp {
	bool operator()(const int & x, const int & y) { return x > y; }
};

int closest(int value, const std::list<int> & vec)
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

int quantize(int n, std::list<int>& s) {

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