#include <cmath>
#include "AverageNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"

using namespace cinder::log;

AverageNode::AverageNode(const Format & format) :
	Node(format)
{
	mAccumBuffer = std::make_shared<BufferDynamic>(mMaxAverage, getNumChannels());
	CI_LOG_I("CHANNELS "<<getNumChannels());
	for (auto& v : mAccum) v = 0.0;
}

AverageNode::~AverageNode()
{
}

void AverageNode::initialize()
{

}

void AverageNode::process(ci::audio::Buffer * buffer)
{

	float readValue, powValue, sqrValue = 0;
	const int numChannels = buffer->getNumChannels();
	const int accumChannels = mAccumBuffer->getNumChannels();
	const int totalChannels = std::min(accumChannels, numChannels);
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	const size_t accumFrames = mAccumBuffer->getNumFrames();
	const size_t accumSize = std::min(accumFrames, mAverage);
	float *accumData = mAccumBuffer->getData();
	float *bufferData = buffer->getData();

	int readCount = 0;
	
	while (readCount < bufferFrames) {


		if (mAccumCurSize > 0) {
			for (int ch = 0; ch < totalChannels; ch++) {
				size_t accumLookup = ch * accumFrames + mAccumPosition;
				size_t bufferLookup = ch * bufferFrames + readCount;
				float oldVal = accumData[accumLookup];
				mAccum[ch] -= accumData[accumLookup];
				readValue = bufferData[bufferLookup];
				powValue = std::pow(readValue, 2);
				accumData[accumLookup] = powValue;
				mAccum[ch] += powValue;
				sqrValue = std::sqrt(mAccum[ch] / mAccumCurSize);
				bufferData[bufferLookup] = sqrValue * mMultiplier;
			}
		}

		if (mAccumCurSize < accumFrames) mAccumCurSize++;
		mAccumPosition++;
		if (mAccumPosition >= accumFrames) mAccumPosition = 0;
		readCount++;
		//CI_LOG_I(accumData[0]);
	
	}
	
}