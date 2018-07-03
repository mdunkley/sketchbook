#include <stdio.h>
#include "GrainNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace std;

namespace cinder {

	namespace audio {

		inline float interpLinear(const float *array, size_t arraySize, float readPos)
		{
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1) % arraySize;
			float val1 = array[index1];
			float val2 = array[index2];
			float frac = readPos - (float)index1;

			return val2 + frac * (val2 - val1);
		}

		inline float interpCosine(const float *array, size_t arraySize, float readPos)
		{
			size_t index1 = (size_t)readPos;
			size_t index2 = (index1 + 1) % arraySize;
			float val1 = array[index1];
			float val2 = array[index2];
			float frac = readPos - (float)index1;

			float mu2 = (1 - cos(frac*M_PI)) / 2;
			return(val1*(1 - mu2) + val2 * mu2);
		}
	}
}


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
	}
}

float Grain::sampleAudio( int channel )
{
	//float panMult = ci::audio::interpLinear(mPanLookup->getData(), mPanSize, size_t((pan + (channel==0 ? 0.5f : 0.0f))*mPanSize));
	//if(channel==0) ci::app::console() << panMult << std::endl;
	//if (channel == 0) ci::app::console() << size_t(ci::clamp((pan + (channel == 0 ? 0.0f : 0.5f)),0.0f,1.0f)*mPanSize) << std::endl;

	return ci::audio::interpLinear(mBuffer->getData(), mBufferSize, channel*mBufferSize + position);
}

float Grain::sampleEnvelope() {

	float nage = age / float(life-1);
	return ci::audio::interpLinear(mEnvelope->getData(), mEnvelope->getSize(), nage*mEnvelope->getSize());
}


void GrainNode::initialize()
{
	// Envelope lookup buffer
	mEnvelope = make_shared<ci::audio::BufferDynamic>();
	mEnvelope->setSize(512,1);
	setEnvelope(GrainEnvelopeType::hann);

	// Pan lookup buffer
	mPanLookup = make_shared<ci::audio::BufferDynamic>();
	mPanLookup->setSize(512, 1);
	calculatePanLookup();

	// Initialize grains
	for (int i=0; i < mGrains.size(); i++) {
		auto grain = new Grain();
		grain->setEnvelope(mEnvelope);
		grain->setPanLookup(mPanLookup);
		grain->active = false;
		mGrains[i] = grain;
	}
}

void GrainNode::calculatePanLookup() {

	size_t panSize = mPanLookup->getNumFrames();
	size_t halfPan = panSize / 2;

	float *data = mEnvelope->getData();

	for (int i = 0; i < panSize; i++) {
		if (i < halfPan) data[i] = glm::sqrt(i / float(panSize));
		else data[i] = glm::sqrt((halfPan - (i - halfPan)) / float(panSize));
	}

	for (int i = 0; i < panSize; i++) {
		ci::app::console() << data[i] << std::endl;
	}
}

void GrainNode::setEnvelope(GrainEnvelopeType type) 
{
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
			data[i] = .5 - (.5 * cos(2 * M_PI* float(i) / (envelopeSize)));
		}
		break;

	case GrainEnvelopeType::hamming:

		for (int i = 0; i < envelopeSize; i++) {
			data[i] = .54 - (.46 * cos(2 * M_PI* float(i) / (envelopeSize)));
		}
		break;
	}
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
	float transRatio = exp(.057762265 * mInterval);

	grain->setBuffer(mBuffer);

	grain->age = 0;
	grain->life = size_t(abs(mLength + mLengthJitter * Rand::randFloat(-1, 1))*sampleRate);  // expressed in grain as samples
	grain->position = size_t(abs(mPosition*bufferLength + mPositionJitter * Rand::randFloat(-1, 1)*sampleRate));
	//grain->rate = (mRate + mRateJitter * Rand::randFloat(-1, 1)) * transRatio;  // Rate multiplied against interval ratio
	//grain->pan = ci::clamp((mPan+1)*.5f + mPanJitter * Rand::randFloat(-1, 1),0.0f,1.0f);  // Player wants 0-1 but we express it in -1,1
	//grain->volume = mVolume + mVolumeJitter * Rand::randFloat(-1, 1);
	grain->active = true;

	//mActiveGrains++;

}

void GrainNode::tick() {

	trigger();
	mTimer = 0;
	mNextTick = abs(mTriggerSpeed
				+ (mTriggerSpeedJitter*Rand::randFloat(-1, 1))) 
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
				//ci::app::console() << envelope << std::endl;	
				
				// Sample grain audio per channel, mult it against the envelope and add to the accumulator
				for (int ch = 0; ch < numChannels; ch++) {
					
					//mChannelAccum[ch] += grain->sampleAudio(ch)*envelope;
					mChannelAccum[ch] += grain->sampleAudio(ch);// *envelope;
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

