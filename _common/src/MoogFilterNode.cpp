#include "MoogFilterNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"

using namespace ci;
using namespace std;

void MoogFilterNode::initialize()
{
}

void MoogFilterNode::process(ci::audio::Buffer *buffer)
{
	const size_t numFrames = buffer->getNumFrames();
	float *data = buffer->getData();

	for (size_t i = 0; i < numFrames; i++) {

		size_t channels = buffer->getNumChannels();
		for (size_t ch = 0; ch<channels; ch++) {
			
			data[ch*numFrames + i] = filter(data[ch*numFrames + i]);
			//data[ch*numFrames + i] = data[ch*numFrames + i];
		}
	}
}

double MoogFilterNode::filter( double input ) {
	double f = mFreq * 1.16;
	double fb = mRes * (1.0 - 0.15 * f * f);
	double in = input;
	in -= mOut4 * fb;
	in *= 0.35013 * (f*f)*(f*f);
	mOut1 = in + 0.3 * mIn1 + (1 - f) * mOut1; // Pole 1
	mIn1 = in;
	mOut2 = mOut1 + 0.3 * mIn2 + (1 - f) * mOut2; // Pole 2
	mIn2 = mOut1;
	mOut3 = mOut2 + 0.3 * mIn3 + (1 - f) * mOut3; // Pole 3
	mIn3 = mOut2;
	mOut4 = mOut3 + 0.3 * mIn4 + (1 - f) * mOut4; // Pole 4
	mIn4 = mOut3;
	return mOut4;
}
