//
//  GranularVoice.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//

#ifndef GranularPlayer_hpp
#define GranularPlayer_hpp

#include <stdio.h>
#include "SampleVoice.hpp"
#include "cinder/audio/audio.h"
#include "cinder/Rand.h"
#include "cinder/Timer.h"
#include "cinder/CinderMath.h"
#include "cinder/app/App.h"
#include "Sequencer.hpp"

using namespace ci;
using namespace std;

class GranularPlayer : public Instrument {
    
public:

    GranularPlayer();
    
    void update() override;
    void disable() override;
    
    void setName( string n ) override;
    
    void setNumSampleVoices( int v  );
    
    void setBuffer( ci::audio::BufferRef buffer );
    void loadBuffer( string sourceFile );
    
    void trigger() override;
    void gate( bool v ) override;
    void choke( float v = .002f ) override;
    
    bool hasTriggered() const { return mHasTriggered; }
    const std::list<SampleVoiceRef>& getTriggeredVoices() const { return mTriggeredVoices; }
    const SampleVoiceRef& getVoice( int voice_number ) { return mSampleVoices.at( voice_number ); }
    
    float getPosition() const { return mPosition; }
    void setPosition(float v) { mPosition = v; }

    float getPositionJitter() const { return mPositionJitter; }
    void setPositionJitter( float ms ) { mPositionJitter = ms; }
    
    float getDuration() const { return mDuration; }
    void setDuration( float s ) { mDuration = s; }

    float getDurationJitter() const { return mDurationJitter; }
    void setDurationJitter( float s ) { mDurationJitter = s; }
    
    float getVolumeJitter() const { return mVolumeJitter; }
    void setVolumeJitter( float v ) { mVolumeJitter = v; }
    
    float getPanJitter() const { return mPanJitter; }
    void setPanJitter( float v  ) { mPanJitter = v; }
    
    float getTriggerSpeed() const { return mSequencer->getRate(); }
    void setTriggerSpeed( float ms ) { mSequencer->setRate(ms); }

    float getVTriggerSpeedJitter() const { return mSequencer->getRateJitter(); }
    void setTriggerSpeedJitter( float v ) {mSequencer->setRateJitter(v); }
    
    float getEnvelopeBias() const { return mEnvelopeBias; }
    void setEnvelopeBias( float v ) { mEnvelopeBias = ci::clamp(v,0.0f,1.0f); }

    float getEnvelopeBiasJitter() const { return mEnvelopeBiasJitter; }
    void setEnvelopeBiasJitter( float v ) { mEnvelopeBiasJitter = ci::clamp(v,-1.0f,1.0f); }

	// Pitch related members
	float getInterval() const { return mInterval; }
	void setInterval(float interval) { mInterval = interval; }

	float getRate() const { return mRate; }
	void setRate(float rate) { mRate = rate; }

    
private:
    
    void triggerVoice();
    
    int mNumSampleVoices = 32;
    int mActiveSampleVoices = 0;

	float mInterval = 0;
	float mRate = 1.0f;

    float mPosition = 0.0f;
    float mPositionJitter = 0.0f;
    float mDuration = .1f;
    float mDurationJitter = 0.0f;
    float mVolumeJitter = 0.0f;
    float mPanJitter = 0.1f;
    float mTriggerSpeed = .1f;
    float mTriggerSpeedJitter = 0.0f;
    float mTriggerSpeedJitterVal = 0;
    
    float mEnvelopeBias = 0.5f;
    float mEnvelopeBiasJitter = 0.0f;
    
    bool mHasTriggered = false;
    bool mGate = false;


    ci::audio::BufferRef mBuffer;
    std::vector<SampleVoiceRef> mSampleVoices;
    std::list<SampleVoiceRef> mTriggeredVoices;
	SequencerRef mSequencer;
    
    size_t mBufferLength;
};

#endif /* GranularPlayer_hpp */
