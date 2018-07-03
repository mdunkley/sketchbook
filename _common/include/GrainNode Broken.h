#pragma once

#include <array>
#include "cinder/Cinder.h"
#include "cinder/audio/Node.h"
#include "cinder/CinderMath.h"

typedef std::shared_ptr<class GrainNode> GrainNodeRef;

class Grain {

public:

	Grain();
	~Grain();

	void setBuffer(ci::audio::BufferRef buffer) { mBuffer = buffer; mBufferSize = buffer->getNumFrames(); }
	void setEnvelope(ci::audio::BufferDynamicRef buffer) { mEnvelope = buffer; mEnvSize = buffer->getNumFrames(); }
	void setPanLookup(ci::audio::BufferDynamicRef buffer) { mPanLookup = buffer; mPanSize = buffer->getNumFrames(); }

	float sampleAudio(int channel);
	float sampleEnvelope();
	void update();

	bool	active;
	double	position;
	float	pan;
	float	volume;
	float	rate;
	size_t	age;
	size_t	life;

private:

	ci::audio::BufferRef mBuffer;
	ci::audio::BufferDynamicRef mEnvelope;
	ci::audio::BufferDynamicRef mPanLookup;

	size_t mBufferSize = 0;
	size_t mEnvSize = 0;
	size_t mPanSize = 0;

};

class GrainNode : public ci::audio::Node {
public:

	GrainNode(const Format &format = Format()) : Node(format) {}

	enum GrainEnvelopeType { constant, hann, hamming };

	void setBuffer(ci::audio::BufferRef buffer) { mBuffer = buffer; }
	void calculatePanLookup();
	void setEnvelope(GrainEnvelopeType type);

	void setTriggerSpeed(float speed) { mTriggerSpeed = speed; }
	float getTriggerSpeed() const { return mTriggerSpeed; }
	void setTriggerSpeedJitter(float jitter) { mTriggerSpeedJitter = jitter; }
	float getTriggerSpeedJitter() const { return mTriggerSpeedJitter; }
	void setDensity(float density) { mTriggerSpeed = 1.0f/density; }
	float getDensity() const { return 1.0f/mTriggerSpeed; }

	void setPosition(float position) { mPosition = position; }
	float getPosition() const { return mPosition; }
	void setPositionJitter(float jitter) { mPositionJitter = jitter; }
	float getPositionJitter() const { return mPositionJitter; }

	void setInterval(float interval) { mInterval = interval; }
	float getInterval() const { return mInterval; }

	void setRate(float rate) { mRate = rate; }
	float getRate() const { return mRate; }
	void setRateJitter(float jitter) { mRateJitter = jitter; }
	float getRateJitter() const { return mRateJitter; }

	void setLength(float Length) { mLength = Length; }
	float getLength() const { return mLength; }
	void setLengthJitter(float jitter) { mLengthJitter = jitter; }
	float getLengthJitter() const { return mLengthJitter; }

	void setPan(float Pan) { mPan = ci::clamp(Pan,-1.0f,1.0f); }
	float getPan() const { return mPan; }
	void setPanJitter(float jitter) { mPanJitter = jitter; }
	float getPanJitter() const { return mPanJitter; }

	void setVolume(float Volume) { mVolume = Volume; }
	float getVolume() const { return mVolume; }
	void setVolumeJitter(float jitter) { mVolumeJitter = jitter; }
	float getVolumeJitter() const { return mVolumeJitter; }

	void gate(bool g) { mGate = g; if(g)tick(); }

	void trigger(double when);
	void trigger();

protected:

	void initialize()							override;

	void process(ci::audio::Buffer *buffer)		override;
	void initializeGrain(Grain* grain);

	void tick();

private:

	std::array<double, 8> mChannelAccum;
	std::array<Grain*, 64> mGrains;

	ci::audio::BufferRef mBuffer;
	ci::audio::BufferDynamicRef mEnvelope;	
	ci::audio::BufferDynamicRef mPanLookup;

	size_t mActiveGrains = 0;
	size_t mTimer = 0;
	size_t mNextTick = 0;

	bool mGate = false;

	float mTriggerSpeed = .01f;
	float mTriggerSpeedJitter = 0.0f;
	float mPosition = 0.0f;
	float mPositionJitter = 0.0f;
	float mInterval = 0.0f;
	float mRate = 1.0f;
	float mRateJitter = 0.0f;
	float mLength = 0.4f;
	float mLengthJitter = 0.0f;
	float mPan = 0.0f;
	float mPanJitter = 0.0f;
	float mVolume = 1.0f;
	float mVolumeJitter = 0.0f;

	friend class PlayerApp;

};
