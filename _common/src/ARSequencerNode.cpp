#include <cmath>
#include "ARSequencerNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"

using namespace cinder::log;

ARSequencerNode::ARSequencerNode(const Format & format) :
	Node(format),
	mThreshold(this),
	mPosition(this)
{
}

ARSequencerNode::~ARSequencerNode()
{
}

void ARSequencerNode::initialize()
{

}

void ARSequencerNode::process(ci::audio::Buffer * buffer)
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
			float trigger = mTrigDetect.process(bufferData[bufferLookup]);

			if( threshData ) bufferData[bufferLookup] = bufferData[bufferLookup] > threshData[readCount];
			else bufferData[bufferLookup] = bufferData[bufferLookup] > mThreshold.getValue();
			
		}
		readCount++;
	}	
}