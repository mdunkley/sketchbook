#include <stdio.h>
#include "SampleNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "CommonUtils.h"
#include "circuits.h"

using namespace ci;
using namespace std;

SampleGrain::SampleGrain()
{
}

SampleGrain::~SampleGrain()
{
}

void SampleGrain::update() {
	position += rate;

	age++;
	if (age >= life) {
		active = 0;
		parent->decrementActiveVoices();
	}
}

float SampleGrain::getAudio( int channel )
{
	int bufferSize = mBuffer->getNumFrames();
	return ci::audio::interpCosineByChannel(mBuffer->getData(), channel, bufferSize, position)*volume;
}

float SampleGrain::getPan(int channel) {
	return ci::audio::interpLinearByChannel(mPanLookup->getData(), channel, mPanLookup->getNumFrames(), pan*mPanLookup->getNumFrames());
}

float SampleGrain::getEnvelope() {

	float edge = 1.0;
	if (age < rampLength) edge = age * rampInc;
	else if (age > (life - rampLength)) edge = rampInc * (life - age);
	
	float nage = age / float(life);
	float lookup = ci::audio::interpLinear(mEnvelope->getData(), mEnvSize, nage*mEnvSize);
	return edge * lookup;
}

void SampleNode::initialize()
{
	// Calc envelope lookup
	calcEnvelope(SampleEnvelopeType::constant);
	calcPanLookup();
	calcNoteLookup();

	// Initialize Samples
	for (int i = 0; i < mSamples.size(); i++) {

		auto sample = new SampleGrain();

		sample->setEnvelope(mEnvelope);
		sample->setPanLookup(mPanLookup);
		sample->active = false;
		mSamples[i] = sample;

	}
}

void SampleNode::calcPanLookup() {

	mPanLookup = make_shared<ci::audio::BufferDynamic>();
	mPanLookup->setSize(512, 2);

	size_t panSize = mPanLookup->getSize();
	size_t halfPan = mPanLookup->getNumFrames();

	float *data = mPanLookup->getData();

	for (int i = 0; i < panSize; i++) {
		if (i < halfPan) data[i] = glm::sqrt(i / float(panSize));
		else data[i] = glm::sqrt((halfPan - (i - halfPan) - 1) / float(panSize));
	}

}

void SampleNode::calcNoteLookup() {

	bool quant = mScale.size();

	for (int i = 0; i < 256; i++) {
		int note = quantize(i-128, mScale);
		mNoteLookup[i] = exp(.057762265*note);
	}

}

void SampleNode::setBuffer(ci::audio::BufferRef buffer)
{
	mBuffer = buffer;
	mBufferChannels = buffer->getNumChannels();
	ci::app::console() << "New buffer has " << mBufferChannels << " channels" << std::endl;
	ci::app::console() << "New bufer is " << buffer->getNumFrames() << " samples long" << std::endl;
}

void SampleNode::calcEnvelope(SampleEnvelopeType type, float modifier)
{
	mEnvelope = make_shared<ci::audio::BufferDynamic>();
	mEnvelope->setSize(1024, 1);
	size_t envelopeSize = mEnvelope->getNumFrames();
	float incr = 1.0 / envelopeSize;
	float *data = mEnvelope->getData();

	switch (type)
	{
	case SampleEnvelopeType::constant:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = 1;
		}
		break;

	case SampleEnvelopeType::hann:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = .5 - (.5 * cos(2 * M_PI* float(i) / (envelopeSize - 1)));
		}
		break;

	case SampleEnvelopeType::hamming:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = .54 - (.46 * cos(2 * M_PI* float(i) / (envelopeSize - 1)));
		}
		break;

	case SampleEnvelopeType::rampUp:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = i * incr;
		}
		break;

	case SampleEnvelopeType::rampDown:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = 1.0-(i * incr);
		}
		break;
	}


}

void SampleNode::setScale(std::list<int> scale)
{
	mScale = scale;
	calcNoteLookup();
}

void SampleNode::trigger()
{
	int count = 0;
	for (auto& Sample : mSamples) {
		if (!Sample->active) {
			initializeSample(Sample);
			break;
		}
		count++;
	}
}

void SampleNode::initializeSample(SampleGrain* sample) 
{
	size_t sampleRate = ci::audio::master()->getSampleRate();
	size_t bufferLength = mBuffer->getNumFrames();

	int interval = int(mInterval + mIntervalJitter * Rand::randFloat());
	float transRatio = mNoteLookup[ interval+128 ];

	if (mPositionValues != nullptr) 
		sample->position = size_t(abs(
			wrap( mPositionValues[mProcessReadCount] * bufferLength, 0, bufferLength ) + 
			mPositionJitter * Rand::randFloat(-1, 1) * sampleRate));
	else
		sample->position = size_t( abs(mPosition.getValue() * bufferLength + 
			mPositionJitter * Rand::randFloat(-1, 1) * sampleRate));

	if (mRateValues != nullptr)
		sample->rate = (mRateValues[mProcessReadCount] + 
			mRateJitter * Rand::randFloat(-1, 1)) * transRatio;
	else
		sample->rate = (mRate + mRateJitter * Rand::randFloat(-1, 1)) * transRatio;

	sample->age = 0;
	sample->life = size_t(abs(mLength + mLengthJitter * Rand::randFloat(-1, 1))*sampleRate);
	sample->rampLength = std::min(mRamp * sampleRate,(float)sample->life*.5f);
	sample->rampInc = 1.0 / sample->rampLength;

	sample->volume = mVolume + mVolumeJitter * Rand::randFloat(-1, 1);
	sample->pan = ci::clamp(mPan + mPanJitter * Rand::randFloat(-1, 1), 0.0f, 1.0f);
	sample->setBuffer(mBuffer);
	sample->setPanLookup(mPanLookup);
	sample->parent = this;
	sample->active = true;

	mActiveVoices++;
}



void SampleNode::tick() {

	trigger();
	mTimer = 0;
	mNextTick = abs(mTriggerRate
				+ (mTriggerRateJitter*Rand::randFloat(-1, 1))) 
				* ci::audio::master()->getSampleRate();

}

void SampleNode::process(ci::audio::Buffer *buffer)
{

	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	size_t numFrames = frameRange.second - frameRange.first;
	float *data = buffer->getData();

	mPositionValues = nullptr;
	if (mPosition.eval()) 
		mPositionValues = mPosition.getValueArray();

	mRateValues = nullptr;
	if (mRateInput.eval())
		mRateValues = mRateInput.getValueArray();

	float const *triggerData = nullptr;
	if (mTriggerInput.eval())
		triggerData = mTriggerInput.getValueArray();

	float const *gateData = nullptr;
	if (mGateInput.eval())
		gateData = mGateInput.getValueArray();

	int readCount = 0;
	int foundVoices = 0;

	while (readCount < numFrames) {

		mProcessReadCount = readCount;

		// Audio Rate trigger setup
		if (triggerData) {

			float newValue = triggerData[readCount];
			float offset = newValue - mOldTriggerValue;
			if (offset < 0) mWaitingForTriggerEdge = true;
			if (mWaitingForTriggerEdge && offset > 0) {
				mWaitingForTriggerEdge = false;
				tick();
			}
			mOldTriggerValue = newValue;
		}

		if (gateData) {
			int gateValue = gateData[readCount] > 0;
			mGate = gateValue;
		}

		// Update timer if gated
		if (mGate) {
			if (mTimer >= mNextTick) tick();
			mTimer++;
		}

		// Loop through all Sample Objects
		for (auto& sample : mSamples) {
			
			// If Sample is running
			if (sample->active) {
				
				// Sample the envelope once
				float envelope = sample->getEnvelope();
				
				// Sample Sample audio per channel, mult it against the envelope and add to the accumulator
				float panMult = 1;

				// Mono
				if (mBufferChannels == 1) { 
					float value = sample->getAudio(0)*envelope;
					for (int ch=0; ch<numChannels; ch++) {
						if (numChannels > 1) panMult = sample->getPan(ch);
						mChannelAccum[ch] += value * panMult;
					}

				// Stereo
				} else {  
					for (int ch = 0; ch < std::max(numChannels,int(mBufferChannels)); ch++) {
						if (numChannels > 1) panMult = sample->getPan(ch);
						mChannelAccum[ch] += sample->getAudio(ch)*envelope*panMult;
					}
				}

				// Increment Sample state
				sample->update();

			}

		}

		// write accumulated values into audio output buffer and zero the accumulator
		for (int ch = 0; ch < numChannels; ch++) {

			//ci::app::console() << mChannelAccum[ch] << std::endl;
			data[ch * numFrames + readCount] = mChannelAccum[ch];
			mChannelAccum[ch] = 0;

		}

		// step to next sample
		readCount++;
	}
}










