//
//  GranularVoice.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//

#ifndef PolySamplePlayer_hpp
#define PolySamplePlayer_hpp

#include <stdio.h>
#include "SampleVoice.hpp"
#include "cinder/audio/audio.h"
#include "cinder/Rand.h"
#include "cinder/Timer.h"
#include "cinder/CinderMath.h"
#include "cinder/app/App.h"
#include "SampleBank.h"


using namespace ci;
using namespace std;


class PolySamplePlayer : public Instrument {
    
public:
	
    PolySamplePlayer();

    void update() override;
    void disable() override;
    
    void setName( string n ) override;
    
    void setNumSampleVoices( int v  );
    
    void setBuffer( ci::audio::BufferRef buffer );
	void setBuffer(ci::audio::BufferRef buffer, BufferInfoRef& info);
    void loadBuffer( string sourceFile );
	BufferInfoRef& getBufferInfo() { return mBufferInfo; }
	ci::audio::BufferRef& getBuffer() { return mBuffer; }
    
    void trigger() override;
    void gate( bool v ) override;
    void choke( float v = .002f ) override;
	virtual void resetParameters() override;

	void setProbabililty(float prob) { mProbability = 1.0f; }
	void setReverseProbability(float prob) { mReverseProb = prob; }
	
	void setSliceDivisions(int div);
	void setToSlicePosition(int slice);
	void setSliceList(std::vector<double>& sliceList);
	void setSlice(int slice);
	void setSliceSpread(float spread) { mSliceSpread = ci::clamp(spread, 0.0f, 1.0f); }
	void setSliceSpreadProbability(float spreadProb) { mSpreadProb = ci::clamp(spreadProb, 0.0f, 1.0f); }
	float getSliceSpreadProbability() const { return mSpreadProb; }
	float getSliceSpread() const { return mSliceSpread; }
	void sliceIncrement();
	void sliceDecrement();
	void sliceWalk();
	size_t  getNumSlices();
	void setSliceLookup(std::map<int, int>& lookup);
	void clearSliceLookup() { mSliceLookup.clear(); }
    
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

	// Pitch related members
	float getInterval() const { return mInterval; }
	void setInterval(float interval) { mInterval = interval; }
	float mInterval = 0;
	float getRate() const { return mRate; }
	void setRate(float rate) { mRate = rate; }
	float mRate = 1.0f;


	// Slice setup
	std::vector<double> mSlices;
	int mCurrentSlice = 0;
	float mSliceSpread = 0.0f;  // 0-1
	int mCuedSlice = 0;
    
    void triggerVoice( const SampleVoiceRef& voice);

	std::map<int, int> mSliceLookup;
	std::vector<int> mLookupKeys;
	std::vector<float> mTransients;

	BufferInfoRef mBufferInfo;
    
    int mNumSampleVoices = 32;
    int mActiveSampleVoices = 0;
    
    float mPosition = 0.0f;
    float mPositionJitter = 0.0f;
    float mDuration = .1f;
    float mDurationJitter = 0.0f;
    float mVolumeJitter = 0.0f;
    float mPanJitter = 0.1f;
    float mTriggerSpeed = .1f;
    float mTriggerSpeedJitter = 0.0f;
    float mTriggerSpeedJitterVal = 0.0f;
	float mProbability = 1.0f; // 0-1
	int	mSliceDivisions = 0;
	float mSpreadProb = 1.0f;
    bool mHasTriggered = false;
    bool mGate = false;
	float mReverseProb = 0.0f;
		
    ci::audio::BufferRef mBuffer;
    std::vector<SampleVoiceRef> mSampleVoices;
    std::list<SampleVoiceRef> mTriggeredVoices;
    
    size_t mBufferLength;
};

#endif /* PolySamplePlayer_hpp */
