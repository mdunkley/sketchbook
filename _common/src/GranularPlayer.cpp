//
//  GranularPlayer.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//

#include "GranularPlayer.hpp"
#include "cinder/Log.h"

GranularPlayer::GranularPlayer(){

	mSequencer = make_shared<Sequencer>();
	setTriggerSpeed(.01);
	mSequencer->getClockSignal().connect([&](StepInfo m) { triggerVoice(); });
	mSequencer->start();
	setNumSampleVoices(64);
	mGainRef->setValue(1);
	enable();
}


void GranularPlayer::setNumSampleVoices( int v  ){
    
    int dif = std::max(1,v)-int(mSampleVoices.size());
    if( dif>0 ){
        for(int i=0; i<dif; i++ ){
            auto s = SampleVoiceRef( new SampleVoice );
            s->setName( mName + "_voice_" + std::to_string(i) );
            s->disable();
			s->getBusOutput() >> getBusInput();
            mSampleVoices.push_back( s );
        }
    } else if( dif<0 ){
        for(int i=0; i<std::abs(dif); i++){
            mSampleVoices.pop_back();
        }
    }
    mNumSampleVoices = v;
}

void GranularPlayer::setName( string n ){
    mName = n;
    for( int i =0; i<mSampleVoices.size(); i++ ){
        mSampleVoices[i]->setName(mName + "_voice_" + std::to_string(i));
    }
}

void GranularPlayer::choke( float v )
{
	Instrument::choke( v );
	
	for( auto& voice : mSampleVoices ) {
		if( voice->isEnabled() )
			voice->choke();
	}
}

void GranularPlayer::gate( bool g ){

    mGate = g;
    if( g ){
		setEnabled(true);
		mSequencer->start();
        //triggerVoice();
	}
	else {
		mSequencer->stop();
	}
	//Instrument::gate(g);
}

void GranularPlayer::update(){
    
    mHasTriggered = false;
    mTriggeredVoices.clear();
	mSequencer->update();
    // Update passthrough for voice self-monitoring
	for( auto& voice : mSampleVoices ) {
		voice->update();
    }

	Instrument::update();

}

void GranularPlayer::disable()
{
	for( auto& voice : mSampleVoices ) {
		voice->disable();
	}
	Instrument::disable();
}


void GranularPlayer::setBuffer( ci::audio::BufferRef buffer )
{
	mBuffer = buffer;
	mBufferLength = buffer->getSize();
}

void GranularPlayer::loadBuffer( string sourceFile ){

    audio::SourceFileRef fileRef = audio::load( loadFile( sourceFile ) );
    setBuffer( fileRef->loadBuffer() );
}

void GranularPlayer::trigger(){

	triggerVoice();
	Instrument::trigger();
}

void GranularPlayer::triggerVoice(){

	bool spawn = false;
	for (auto& voice : mSampleVoices) {
		if (!voice->isEnabled()) {

			float envJitter = Rand::randFloat();
			float env = ci::clamp( float (mEnvelopeBias + envJitter*mEnvelopeBiasJitter ), 0.0f, 1.0f ) ;
			voice->setAttack( std::max(.002f,mDuration*env)  );
			voice->setRelease( std::max(.002f,mDuration*(1-env)) );
			voice->setInterval(mInterval);
			voice->setRate(mRate);
			voice->setPan( mPan+mPanJitter*(Rand::randFloat()-.5f) );
			voice->setPosition( ci::clamp(mPosition + mPositionJitter * Rand::randFloat(), 0.0f, 1.0f) );
			voice->setBuffer(mBuffer);
			voice->trigger();

			mTriggeredVoices.push_back(voice);

			break;
		}
	}
	mHasTriggered = spawn;
}


