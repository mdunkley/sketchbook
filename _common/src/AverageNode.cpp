#include "AverageNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"



AverageNode::AverageNode(const Format & format) :
	Node(format)
{
	mAccumBuffer = std::make_shared<BufferDynamic>(mMaxAverage, getNumChannels());;
}

AverageNode::~AverageNode()
{
}

void AverageNode::initialize()
{

}

void AverageNode::process(ci::audio::Buffer * buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	const size_t accumFrames = mAccumBuffer->getNumFrames();
	float *accumData = mAccumBuffer->getData();
	float *bufferData = buffer->getData();

	int readCount = 0;
	/*
	while (readCount < bufferFrames) {

		for (int ch = 0; ch < numChannels; ch++) {
			size_t accumPos = ch * accumFrames + mAccumPosition;
			size_t bufferPos = ch * bufferFrames + readCount;
			mAccum -= accumData[accumPos];
			float readValue = bufferData[bufferPos];
			float powValue = glm::pow(readValue, 2);
			accumData[accumPos] = powValue;
			mAccum += powValue;
			float squaredVal = glm::sqrt(mAccum / mAccumCurSize);
			bufferData[bufferPos] = squaredVal;
		}
		mAccumCurSize = glm::clamp(++mAccumCurSize, (size_t)1, accumFrames);
		mAccumPosition = wrap(mAccumPosition + 1, 0, accumFrames);
		readCount++;
	
	}
	*/
}