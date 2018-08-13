#include <cmath>
#include "ComparatorNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"

using namespace cinder::log;

ComparatorNode::ComparatorNode(const Format & format) :
	Node(format),
	mThreshold(this)
{
	setChannelMode(ci::audio::Node::ChannelMode::SPECIFIED);
	setNumChannels(1);
}

ComparatorNode::~ComparatorNode()
{
}

void ComparatorNode::initialize()
{

}

void ComparatorNode::process(ci::audio::Buffer * buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	float *bufferData = buffer->getData();

	const float* threshData = nullptr;
	if (mThreshold.eval()) threshData = mThreshold.getValueArray();

	int readCount = 0;
	
	while (readCount < bufferFrames) {

		for (int ch = 0; ch < numChannels; ch++) {

			size_t bufferLookup = ch * bufferFrames + readCount;
			if( threshData ) bufferData[bufferLookup] = bufferData[bufferLookup] > threshData[readCount];
			else bufferData[bufferLookup] = bufferData[bufferLookup] > mThreshold.getValue();
			
		}
		readCount++;
	}	
}