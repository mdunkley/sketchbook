#include <cmath>
#include "SequencerNode.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/audio/Buffer.h"
#include "CommonUtils.h"
#include "Circuits.h"
#include "cinder/gl/gl.h"


using namespace cinder::log;

SequencerNode::SequencerNode(const Format & format) :
	Node(format),
	mPosition(this),
	mSigDelay()
{

	mSigDelay = std::make_shared<Circuits::Delay>();
	mSequence.fill(0);
	mSigDelay->setDelaySize(ci::audio::master()->getSampleRate());
	mSigDelay->setDelaySize(10);
}

SequencerNode::~SequencerNode()
{
}

void SequencerNode::draw()
{
}

void SequencerNode::setSequence(std::vector<float> values)
{
	int size = std::min(values.size(), (size_t)256);
	for (int i=0; i < size; i++) { mSequence[i] = values.at(i); }
	mLength = size;
}

void SequencerNode::initialize()
{

}

void SequencerNode::setDelaySize(size_t delaysize) {
	mSigDelay->setDelaySize(delaysize);
	mDelaySize = delaysize;
}

void SequencerNode::getNextStep() {

	bool r = ci::Rand::randBool();

	switch (mDirection){

		case Direction::up:
			mCurrentStep += 1;
			if (mCurrentStep >= mLength) mCurrentStep -= mLength;
			break;

		case Direction::down:
			mCurrentStep -= 1;
			if (mCurrentStep < 0) mCurrentStep += mLength;
			break;

		case Direction::updown:
			break;

		case Direction::walk:
			if (r) {
				mCurrentStep += 1;
				if (mCurrentStep >= mLength) mCurrentStep -= mLength;
			}
			else {
				mCurrentStep -= 1;
				if (mCurrentStep < 0) mCurrentStep += mLength;
			}
			break;

		case Direction::random:
			mCurrentStep = ci::Rand::randFloat(mLength);
			break;
	}
}

void SequencerNode::process(ci::audio::Buffer * buffer)
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	float *bufferData = buffer->getData();

	const float* posData = nullptr;
	if (mPosition.eval()) posData = mPosition.getValueArray();

	int readCount = 0;
	
	while (readCount < bufferFrames) {

		size_t bufferLookup = readCount;
		float value = bufferData[bufferLookup];
		float trigger = mChange(mTrigDetect(value))>0;
		mTriggerCount += trigger;

		if (mTriggerCount>=mClockDivisions) {
			mTriggerCount = 0;
			if (posData) mCurrentStep = std::floor(posData[readCount] * mLength);
			else getNextStep();
		}

		bufferData[bufferLookup] = mSigDelay->process( mSequence[mCurrentStep] );
		//bufferData[bufferLookup] = mSequence[mCurrentStep];

		readCount++;
	}
}