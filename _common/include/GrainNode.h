#pragma once

#include <list>
#include <array>
#include "cinder/Cinder.h"
#include "cinder/audio/Node.h"

typedef std::shared_ptr<class GrainNode> GrainNodeRef;

class Grain {

public:

	Grain();
	~Grain();

	void setBuffer(ci::audio::BufferRef buffer) { mBuffer = buffer; mBufferSize = buffer->getNumFrames(); }
	void setEnvelope(ci::audio::BufferDynamicRef buffer) { mEnvelope = buffer; mEnvSize = buffer->getNumFrames(); }
	void setPanLookup(ci::audio::BufferDynamicRef buffer) { mPanLookup = buffer; mPanSize = buffer->getNumFrames(); }

	float sampleAudio(int channel);
	float samplePan(int channel);
	float sampleEnvelope();
	void update();

	GrainNode* parent;

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

	friend class GrainNode;

};

class GrainNode : public ci::audio::Node {
public:

	GrainNode(const Format &format = Format()) : Node(format) {}

	enum GrainEnvelopeType { constant, hann, hamming };

	void setBuffer(ci::audio::BufferRef buffer) { mBuffer = buffer; }
	void calcEnvelope(GrainEnvelopeType type);
	void calcPanLookup();
	void calcNoteLookup();

	void setTriggerSpeed(float speed) { mTriggerRate = speed; }
	float getTriggerSpeed() const { return mTriggerRate; }
	void setTriggerSpeedJitter(float jitter) { mTriggerRateJitter = jitter; }
	float getTriggerSpeedJitter() const { return mTriggerRateJitter; }
	void setDensity(float density) { mTriggerRate = 1.0f / density; }
	float getDensity() const { return 1.0f / mTriggerRate; }

	void setPosition(float position) { mPosition = position; }
	float getPosition() const { return mPosition; }
	void setPositionJitter(float jitter) { mPositionJitter = jitter; }
	float getPositionJitter() const { return mPositionJitter; }

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

	int		getNumActiveGrains() { return mActiveGrains; }
	void	decrementActiveGrains() { mActiveGrains--; }

	void	gate(bool g) { mGate = g; if(g)tick(); }
	void	trigger();

protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;
	void	initializeGrain(Grain* grain);

	void	tick();

private:

	std::array<float, 256> mNoteLookup;
	std::array<float, 16> mChannelAccum;
	std::array<Grain*, 256> mGrains;

	std::list<int> mScale;

	ci::audio::BufferRef mBuffer;
	ci::audio::BufferDynamicRef mEnvelope;
	ci::audio::BufferDynamicRef mPanLookup;

	size_t mActiveGrains = 0;
	size_t mTimer = 0;
	size_t mNextTick = 0;

	bool mGate = false;

	float mTriggerRate = .01f;
	float mTriggerRateJitter = 0.0f;
	float mPosition = 0.0f;
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
