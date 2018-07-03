#include "DistortionNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"

using namespace ci;
using namespace std;

void DistortionNode::initialize()
{
	mSampleRate = ci::audio::master()->getSampleRate();
	for (int i = 0; i<8; i++) mChannelValues[i]=0;
}

void DistortionNode::process(ci::audio::Buffer *buffer)
{

	size_t samples = std::max(ci::clamp(float(mSampleRateMult), 0.0f, 1.0f) * 100, 1.0f);
	const int channels = buffer->getNumChannels();
	const size_t numFrames = buffer->getNumFrames();
	float *data = buffer->getData();


	for (size_t i = 0; i < numFrames; i++) {

		for (int ch = 0; ch<channels; ch++) {

			if ((mCount >= samples) || (mCount==0))  {
				mCount = 0;
				mChannelValues[ch] = data[ch*numFrames + i];
			}

			data[ch*numFrames + i] = mChannelValues[ch];

			
			//data[ch*numFrames + i] = std::floor(mChannelValues[ch] * bitMult) * mBitRateMult;


			/*
			// TanH distortion
			sig *= mAmp;
			if (sig<(-mThreshold) || sig>mThreshold) data[ch * numFrames + i] = distort(sig);
			data[ch*numFrames + i] = sig;

			*/

			if (ch == (channels - 1)) mCount++;
		}
	}
}

double DistortionNode::distort(double x) {
	// std::atan(sig);
	// std::tanh(sig);

	//Asym variation
	double a = ((0.0515*x + 0.03899)*x) + x;
	double a2 = a * a;
	return x / sqrt(a2 + 0.6211);
}
