//
//  PolyWavetablePlayer.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//

#include "PolyWavetablePlayer.hpp"

PolyWavetablePlayer::PolyWavetablePlayer(){
    init();
}

PolyWavetablePlayer::PolyWavetablePlayer( AudioManager* a ){
    mParent = a;
    init();
}

void PolyWavetablePlayer::init(){
    enable();
    mGainRef->setValue(1);
    setNumVoices( 8 );
}

void PolyWavetablePlayer::setName( string name )
{
	mName = name;
	int count = 0;
	for( auto& voice : mVoices ) {
		voice->setName( mName + "_voice_" + std::to_string( count ) );
		count++;
	}
}

void PolyWavetablePlayer::setNumVoices( int v  ){
    
    int dif = std::max(1,v)-int(mVoices.size());
    if( dif>0 ){
        for(int i=0; i<dif; i++ ){
            auto s = WavetableVoiceRef( new WavetableVoice() );
            mVoices.push_back( s );
            s->getBusOutput()>>getBusInput();
        }
    } else if( dif<0 ){
        for(int i=0; i<std::abs(dif); i++){
            mVoices.pop_back();
        }
    }
    mNumVoices = v;
    setName( mName );
}

void PolyWavetablePlayer::gate( bool g ){

    if( g ) trigger();
}

void PolyWavetablePlayer::update(){
    
    mHasTriggered=false;
    mTriggeredVoices.clear();
    
    // Update passthrough for voice self-monitoring
	for( auto& voice : mVoices ) {
		voice->update();
    }
}

void PolyWavetablePlayer::disable(){
    
	for( auto& voice : mVoices ) {
		voice->disable();
	}

    //Instrument::disable();
}

void PolyWavetablePlayer::setWaveform( string w ) {
	for( auto& voice : mVoices ) {
		voice->setWaveform( w );
	}
    
}


void PolyWavetablePlayer::trigger(){
    
    int count = 0;
	for( auto& voice : mVoices ) {
        count ++;
        if( ! voice->isEnabled() ){
            triggerVoice( voice );
            break;
        }
    }
    mHasTriggered=true;

}

void PolyWavetablePlayer::triggerVoice( const WavetableVoiceRef& voice )
{
    voice->enable();
    voice->setAttack( std::max(.001f, mAttack + mAttackJitter*( Rand::randFloat()-.5f) ));
    voice->setDecay( std::max(.001f, mDecay + mDecayJitter*( Rand::randFloat()-.5f) ) );
    voice->setPan( mPan + mPanJitter*( Rand::randFloat()-.5f) );
    voice->setFreq( mFreq + mFreqJitter*( Rand::randFloat()-.5f) );
    voice->setVolume( mVolume + mVolumeJitter*( Rand::randFloat()-.5f) );
    voice->trigger();
    mTriggeredVoices.push_back(voice);
}

void PolyWavetablePlayer::setFreqMidi( float freqMidi ) {
    setFreq( ci::audio::midiToFreq( freqMidi ) );
    
}



