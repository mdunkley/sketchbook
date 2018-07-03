//
//  PolyWavetablePlayer.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//
#pragma once

#include <stdio.h>
#include "SampleVoice.hpp"
#include "cinder/audio/audio.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include "cinder/Timer.h"
#include "WavetableVoice.hpp"

using namespace ci;
using namespace std;



class PolyWavetablePlayer : public Instrument {
    
public:
    
    PolyWavetablePlayer();
    PolyWavetablePlayer( AudioManager* a);
    
    void init();
    void update() override;
    void disable() override;
    
    void setNumVoices( int v  );
    void setWaveform( string w );

    void setName( string name ) override;
    
    float   getFreq() const { return mFreq; }
    void    setFreqMidi( float freqMidi );
    void    setFreq( float freqHz ) { mFreq = freqHz; }

    void trigger() override;
    void triggerVoice( const WavetableVoiceRef& voice );
    
    void gate( bool v ) override;
    bool getGate() const {return mGate;}
    
    bool hasTriggered() const { return mHasTriggered; }
    const std::list<WavetableVoiceRef>& getTriggeredVoices() const { return mTriggeredVoices; }
	WavetableVoiceRef getVoice( int voice_number ) { return mVoices.at( voice_number ); }

private:
    
    Timer mTimer;
    
    bool mGate = false;

    int         mActiveVoices = 0;
    int         mNumVoices = 0;
    
    float       mFreq = 0.0f;
    float       mFreqJitter = 0.0;
    float       mPanJitter = 0.0f;
    float       mAttackJitter = 0.0f;
    float       mDecayJitter = 0.0f;
    float       mVolumeJitter = 0.0f;
    
    bool mHasTriggered = false;

    
    std::vector<WavetableVoiceRef> mVoices;
    std::list<WavetableVoiceRef> mTriggeredVoices;
    
};

