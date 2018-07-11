#include <cmath>
#include "TunerNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"

using namespace cinder::log;

TunerNode::TunerNode(const Format & format) :
	Node(format),
	mSampleRate(ci::audio::master()->getSampleRate())
{
	mLookup.fill(0);
}

TunerNode::~TunerNode()
{
}

void TunerNode::initialize()
{

}

void TunerNode::process(ci::audio::Buffer * buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	float *bufferData = buffer->getData();

	float inc = 1.0/ci::audio::master()->getSampleRate()/10;

	int readCount = 0;
	while (readCount < bufferFrames) {

		float thisValue = bufferData[readCount];
		bool thisSign = std::signbit(thisValue);

		mCount++;
		if (thisSign != mPrevSign)
		{
			int newNote = std::round(ci::audio::freqToMidi(float(mSampleRate) / mCount * .5));
			mFrequency = ci::audio::midiToFreq(newNote);
			if (mFrequency != mPrevFrequency) {
				mLookup[mPlayedNote] = mDetectedNote;
				CI_LOG_D(mPlayedNote<<" " << mLookup[mPlayedNote]);
			}
			mDetectedNote = newNote;
			mCount = 0;
			mPrevFrequency = mFrequency;
			mPrevSign = thisSign;
		}

		mAccum += inc;
		if (mAccum >= 1) mAccum = 0;
		mPlayedNote = std::round(mAccum*127.0);
		bufferData[readCount] = ci::audio::midiToFreq(mPlayedNote);

		readCount++;

	}	
}