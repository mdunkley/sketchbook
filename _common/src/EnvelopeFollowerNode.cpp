#include <cmath>
#include "EnvelopeFollowerNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"

using namespace cinder::log;

EnvelopeFollowerNode::EnvelopeFollowerNode(const Format & format) :
	Node(format)
{
	mAccumBuffer = std::make_shared<BufferDynamic>(mMaxInterval, getNumChannels());
	//CI_LOG_I("CHANNELS "<<getNumChannels());
	for (auto& v : mAccum) v = 0.0;
}

EnvelopeFollowerNode::~EnvelopeFollowerNode()
{
}

void EnvelopeFollowerNode::initialize()
{

}

void EnvelopeFollowerNode::process(ci::audio::Buffer * buffer)
{

	float readValue, powValue, sqrValue = 0;
	const int numChannels = buffer->getNumChannels();
	const int accumChannels = mAccumBuffer->getNumChannels();
	const int totalChannels = std::min(accumChannels, numChannels);
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	const size_t accumFrames = mAccumBuffer->getNumFrames();
	const size_t accumSize = std::min(accumFrames, mInterval);
	float *accumData = mAccumBuffer->getData();
	float *bufferData = buffer->getData();

	int readCount = 0;
	
	while (readCount < bufferFrames) {

		if (mAccumCurSize > 0) {
			for (int ch = 0; ch < totalChannels; ch++) {
				size_t accumLookup = ch * accumFrames + mAccumPosition;
				size_t bufferLookup = ch * bufferFrames + readCount;
				//float oldVal = accumData[accumLookup];
				mAccum[ch] -= accumData[accumLookup];
				readValue = bufferData[bufferLookup];
				if (readValue > 0) {
					powValue = std::pow(readValue, 2);
					accumData[accumLookup] = powValue;
					mAccum[ch] += powValue;
				}
				else {
					accumData[accumLookup] = 0;
				}

				double divVal = mAccum[ch] / std::max(mAccumCurSize,(size_t)1);
				if( divVal>.00000001 ) sqrValue = std::sqrt(divVal);
				else sqrValue = 0;
				bufferData[bufferLookup] = sqrValue * mMultiplier;
			}
		}

		if (mAccumCurSize < (accumSize)) mAccumCurSize++;
		mAccumPosition++;
		if (mAccumPosition >= accumSize) mAccumPosition = 0;
		readCount++;
	
	}
	
}