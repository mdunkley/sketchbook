#include <stdio.h>
#include "GrainNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "CommonUtils.h"

using namespace ci;
using namespace std;


Grain::Grain()
{
}

Grain::~Grain()
{
}

void Grain::update() {
	position += rate;
	age++;
	if (age >= life) {
		active = 0;
		parent->decrementActiveGrains();
	}
}

float Grain::sampleAudio( int channel )
{
	int bufferSize = mBuffer->getNumFrames();
	return ci::audio::interpCosineByChannel(mBuffer->getData(), channel, bufferSize, position)*volume;
}

float Grain::samplePan(int channel) {
	return ci::audio::interpLinearByChannel(mPanLookup->getData(), channel, mPanLookup->getNumFrames(), pan*mPanLookup->getNumFrames());
}

float Grain::sampleEnvelope() {

	float nage = age / float(life);
	return ci::audio::interpLinear(mEnvelope->getData(), mEnvSize, nage*mEnvSize);
}

void GrainNode::initialize()
{

	// Calc envelope lookup

	calcEnvelope(GrainEnvelopeType::hann);

	calcPanLookup();
	calcNoteLookup();

	// Initialize grains
	for (int i = 0; i < mGrains.size(); i++) {

		auto grain = new Grain();

		grain->setEnvelope(mEnvelope);
		grain->setPanLookup(mPanLookup);
		grain->active = false;
		mGrains[i] = grain;
	}
}

void GrainNode::calcPanLookup() {

	mPanLookup = make_shared<ci::audio::BufferDynamic>();
	mPanLookup->setSize(512, 2);

	size_t panSize = mPanLookup->getSize();
	size_t halfPan = mPanLookup->getNumFrames();

	float *data = mPanLookup->getData();

	for (int i = 0; i < panSize; i++) {
		if (i < halfPan) data[i] = glm::sqrt(i / float(panSize));
		else data[i] = glm::sqrt((halfPan - (i - halfPan)) / float(panSize));
	}

	for (int i = 0; i < panSize; i++) {
		ci::app::console() << data[i] << std::endl;
	}
}

void GrainNode::calcNoteLookup() {

	bool quant = mScale.size();

	for (int i = 0; i < 256; i++) {

		int note = quantize(i-128, mScale);
		//ci::app::console() << i << " " << note << " " << std::endl;
		mNoteLookup[i] = exp(.057762265*note);
	}

}

void GrainNode::calcEnvelope(GrainEnvelopeType type) 
{
	mEnvelope = make_shared<ci::audio::BufferDynamic>();
	mEnvelope->setSize(512, 1);

	size_t envelopeSize = mEnvelope->getNumFrames();
	float *data = mEnvelope->getData();

	switch (type)
	{
	case GrainEnvelopeType::constant:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = 1;
		}
		break;

	case GrainEnvelopeType::hann:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = .5 - (.5 * cos(2 * M_PI* float(i) / (envelopeSize -1)));
		}
		break;

	case GrainEnvelopeType::hamming:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = .54 - (.46 * cos(2 * M_PI* float(i) / (envelopeSize - 1)));
		}
		break;
	}
}

void GrainNode::setScale(std::list<int> scale)
{
	mScale = scale;
	calcNoteLookup();
}

void GrainNode::trigger()
{
	int count = 0;
	for (auto& grain : mGrains) {
		if (!grain->active) {
			initializeGrain(grain);
			break;
		}
		count++;
	}
}

void GrainNode::initializeGrain(Grain* grain) 
{
	size_t sampleRate = ci::audio::master()->getSampleRate();
	size_t bufferLength = mBuffer->getNumFrames();

	int interval = int(mInterval + mIntervalJitter * Rand::randFloat());
	float transRatio = mNoteLookup[ interval+128 ];
	grain->setBuffer(mBuffer);
	grain->setPanLookup(mPanLookup);
	//grain->setEnvelope(mEnvelope);

	grain->age = 0;
	grain->life = size_t(abs(mLength + mLengthJitter * Rand::randFloat(-1, 1))*sampleRate);
	grain->position = size_t(abs(mPosition*bufferLength + mPositionJitter * Rand::randFloat(-1, 1)*sampleRate));
	grain->rate = (mRate + mRateJitter * Rand::randFloat(-1, 1)) * transRatio;
	grain->volume = mVolume + mVolumeJitter * Rand::randFloat(-1, 1);
	grain->pan = ci::clamp(mPan + mPanJitter * Rand::randFloat(-1, 1), 0.0f, 1.0f);
	grain->parent = this;
	grain->active = true;

	mActiveGrains++;
}



void GrainNode::tick() {
	trigger();
	mTimer = 0;
	mNextTick = abs(mTriggerRate
				+ (mTriggerRateJitter*Rand::randFloat(-1, 1))) 
				* ci::audio::master()->getSampleRate();

}

void GrainNode::process(ci::audio::Buffer *buffer)
{

	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	size_t numFrames = frameRange.second - frameRange.first;
	float *data = buffer->getData();


	int readCount = 0;
	while (readCount < numFrames) {

		// Update timer if gated
		if (mGate) {
			if (mTimer >= mNextTick) tick();
			mTimer++;
		}

		// Loop through all grains
		for (auto& grain : mGrains) {

			// If grain is running
			if (grain->active) {
				
				// Sample the envelope once
				float envelope = grain->sampleEnvelope();
				
				// Sample grain audio per channel, mult it against the envelope and add to the accumulator
				float panValue = 1;
				for (int ch = 0; ch < numChannels; ch++) {
					if (numChannels > 1) panValue = grain->samplePan(ch);
					mChannelAccum[ch] += grain->sampleAudio(ch)*envelope*panValue;
				}

				// Increment grain state
				grain->update();
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










