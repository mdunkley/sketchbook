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
	mLookup.fill(-1);
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

	// increment value
	float inc = 1.0/ci::audio::master()->getSampleRate()/20;

	int readCount = 0;
	while (readCount < bufferFrames) {

		if (mCalibrate) {

			float thisValue = bufferData[readCount];
			bool thisSign = std::signbit(thisValue);

			mCount++;
			if (thisSign != mPrevSign)
			{

				int newNote = ci::clamp( int(std::ceil(ci::audio::freqToMidi(float(mSampleRate) / mCount * .5))),0,127);
				mFrequency = ci::audio::midiToFreq(newNote);

				if ((newNote!=mDetectedNote)&&(mLookup[newNote] == -1)) {

					mLookup[mDetectedNote] = mPlayedValue;

					CI_LOG_D( mDetectedNote << " " << mPlayedValue);
					mDetectedNote = newNote;

				}

				mCount = 0;
				mPrevFrequency = mFrequency;
				mPrevSign = thisSign;
			}

			mAccum += inc;
			if (mAccum >= 1) {
				mCalibrate = false;
			}

			mPlayedValue = ci::lerp(mPlayedValue,mAccum*mAccum*10000.0,1);
			bufferData[readCount] = mPlayedValue;
			//CI_LOG_D(bufferData[readCount]);
		}

		readCount++;

	}	
}

void TunerNode::calibrate()
{
	mCalibrate = true;
	mAccum = 0;
	mPlayedValue = 0;

}
