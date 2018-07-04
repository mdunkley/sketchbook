#pragma once

#include <list>
#include <array>
#include "cinder/Cinder.h"
#include "cinder/audio/Node.h"
#include "cinder/audio/Param.h" 

typedef std::shared_ptr<class SampleNode> SampleNodeRef;

class SampleGrain {

public:

	SampleGrain();
	~SampleGrain();

	void setBuffer(ci::audio::BufferRef buffer) { mBuffer = buffer; mBufferSize = buffer->getNumFrames(); }
	void setEnvelope(ci::audio::BufferDynamicRef buffer) { mEnvelope = buffer; mEnvSize = buffer->getNumFrames(); }
	void setPanLookup(ci::audio::BufferDynamicRef buffer) { mPanLookup = buffer; mPanSize = buffer->getNumFrames(); }

	float getAudio(int channel);
	float getPan(int channel);
	float getEnvelope();
	void update();

	SampleNode* parent;

	double rampLength = 0;  // length in samples
	double rampInc = .000001;

	bool	active = false;
	double	position = 0.0;
	float	pan = .5f;
	float	volume = 1.0f;
	float	rate = 1.0f;
	size_t	age = 0;
	size_t	life;

private:

	ci::audio::BufferRef mBuffer;
	ci::audio::BufferDynamicRef mEnvelope;
	ci::audio::BufferDynamicRef mPanLookup;

	size_t mBufferSize = 0;
	size_t mEnvSize = 0;
	size_t mPanSize = 0;

	friend class SampleNode;

};

class SampleNode : public ci::audio::Node {
public:

	SampleNode(const Format &format = Format()) : Node(format), mPosition(this) {}

	enum SampleEnvelopeType { constant, hann, hamming };

	ci::audio::Param* getPositionParam() { return &mPosition; }

	void setBuffer(ci::audio::BufferRef buffer);
	void calcEnvelope(SampleEnvelopeType type);
	void calcPanLookup();
	void calcNoteLookup();

	void setTriggerSpeed(float speed) { mTriggerRate = speed; }
	float getTriggerSpeed() const { return mTriggerRate; }
	void setTriggerSpeedJitter(float jitter) { mTriggerRateJitter = jitter; }
	float getTriggerSpeedJitter() const { return mTriggerRateJitter; }
	void setDensity(float density) { mTriggerRate = 1.0f / density; setLength(mTriggerRate * 8); }
	float getDensity() const { return 1.0f / mTriggerRate; }

	void setPosition(float phase) { mPosition.setValue( phase ); }
	float getPosition() const { return mPosition.getValue(); }
	void setPositionJitter(float jitter) { mPositionJitter = jitter; }
	float getPositionJitter() const { return mPositionJitter; }

	void setRamp(float length) { mRamp = length; }
	float getRamp() const { return mRamp; }

	void setInterval(float interval) { mInterval = interval; }
	float getInterval() const { return mInterval; }
	void setIntervalJitter(float jitter) { mIntervalJitter = jitter; }
	float getIntervalJitter() const { return mIntervalJitter; }

	void setRate(float rate) { mRate = rate; }
	float getRate() const { return mRate; }
	void	setRateJitter(float jitter) { mRateJitter = jitter; }
	float	getRateJitter() const { return mRateJitter; }

	void	setLength(float length) { mLength = length; }
	float	getLength() const { return mLength; }
	void	setLengthJitter(float jitter) { mLengthJitter = jitter; }
	float	getLengthJitter() const { return mLengthJitter; }

	void	setPan(float pan) { mPan = pan; }
	float	getPan() const { return mPan; }
	void	setPanJitter(float jitter) { mPanJitter = jitter; }
	float	getPanJitter() const { return mPanJitter; }

	void	setVolume(float volume) { mVolume = volume; }
	float	getVolume() const { return mVolume; }
	void	setVolumeJitter(float jitter) { mVolumeJitter = jitter; }
	float	getVolumeJitter() const { return mVolumeJitter; }

	void	setScale(std::list<int> scale);
	std::list<int> getScale() { return mScale; }
	std::array<float,256>& getNoteLookup() { return mNoteLookup; }

	int		getNumActiveVoices() { return mActiveVoices; }
	void	decrementActiveVoices() { mActiveVoices--; }

	void	gate(bool g) { mGate = g; if(g)tick(); }
	void	trigger();

protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;
	void	initializeSample(SampleGrain* Sample);

	void	tick();

private:

	std::array<float, 256> mNoteLookup;
	std::array<float, 16> mChannelAccum;
	std::array<SampleGrain*, 64> mSamples;

	std::list<int> mScale;

	ci::audio::BufferRef mBuffer;
	ci::audio::BufferDynamicRef mEnvelope;
	ci::audio::BufferDynamicRef mPanLookup;

	size_t mActiveVoices = 0;
	size_t mTimer = 0;
	size_t mNextTick = 0;
	size_t mProcessReadCount = 0;
	size_t mBufferChannels = 0;

	bool mGate = false;

	ci::audio::Param mPosition;
	const float *mPositionValues = nullptr;

	float mRamp = .002f;

	float mTriggerRate = .01f;
	float mTriggerRateJitter = 0.0f;
	float mPositionJitter = 0.0f;
	float mInterval = 0.0f;
	float mIntervalJitter = 0.0f;
	float mRate = 1.0f;
	float mRateJitter = 0.0f;
	float mLength = 0.5f;
	float mLengthJitter = 0.0f;
	float mPan = 0.5f;
	float mPanJitter = 0.0f; 
	float mVolume = 1.0f;
	float mVolumeJitter = 0.0f;

	friend class PlayerApp;
};
