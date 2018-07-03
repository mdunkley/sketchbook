//
//  Instrument.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/12/18.
//

#include "Instrument.hpp"
#include "cinder/app/AppBase.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"

Instrument::Instrument()
{
 
    mPanRef = audio::master()->makeNode( new audio::Pan2dNode() );
    mPanRef->setPos( .5 );
    mNodes.push_back(mPanRef);
    mGainRef = audio::master()->makeNode( new audio::GainNode() );
    mGainRef->setValue(0);
    mNodes.push_back(mGainRef);
	mReleaseRamp = mGainRef->getParam()->applyRamp( 0, INT_MAX );
    mMonitorRef = audio::master()->makeNode( new audio::MonitorNode() );
    mGainRef>>mPanRef>>mMonitorRef;
    
    setBusInput( mGainRef );
    setBusOutput( mPanRef );
    
    disable();
    
}

Instrument::~Instrument(){
}

void Instrument::enable(){

    for( auto& n: mNodes ){
        n->enable();
    }
	if (mFxAutoEnable) {
		for (auto& n : mFxNodes) {
			n->enable();
		}
	}
    mActivationTime = audio::master()->getNumProcessedSeconds();
    if( mMonitorActive ) mMonitorRef->enable();
    mEnabled = true;
}

void Instrument::disable(){

    for( auto& n: mNodes ) n->disable();
	for (auto& n : mFxNodes) n->disable();
	
    mMonitorRef->disable();
    mEnabled = false;
	mInUse = false;

}

void Instrument::setEnabled( bool v ){
    if( v ) enable();
    else disable();
}

void Instrument::setMonitorActive( bool v ){
    
    mMonitorActive = v;
    
    if( mMonitorActive ) mMonitorRef->enable();
    else mMonitorRef->disable();

}

float Instrument::getRms() {

    if( !getMonitorActive() ) setMonitorActive( true );
	if (isEnabled()) {

		float rawRMS = mMonitorRef->getVolume() * mRmsMult;
		if (rawRMS > mRmsSlew) {
			mRmsSlew = lerp(mRmsSlew, rawRMS, mRmsSlewAttack);
			mRmsSlew = lerp(mRmsSlew, rawRMS, mRmsSlewAttack);
		}
		else {
			mRmsSlew = lerp(mRmsSlew, rawRMS, mRmsSlewDecay);
		}
		return ci::clamp(mRmsSlew,0.0f,1.0f);

	}
	else return 0.0;

}

double Instrument::getVoiceAge(){

    return audio::master()->getNumProcessedSeconds()-mActivationTime;

}

void Instrument::resetParameters() {

	//CI_LOG_I("Resetting instrument parameters for " << getName());
	mVolume = 1.0f;
	mPan = 0.5f;
	mAttack = 0.002f;
	mAttackJitter = 0.0f;
	mHold = 0.0f;
	mHoldJitter = 0.0f;
	mDecay = 0.002f;
	mDecayJitter = 0.0f;
	mSustain = 1.0f;
	mSustainJitter = 0.0f;
	mRelease = .002f;
	mReleaseJitter = 0.0f;
	mRmsSlewAttack = .96f;
	mRmsSlewDecay = .05f;
	mRmsSlew = 0.0;
	mRmsMult = 10.0f;
}

void Instrument::trigger() {

	if (!isEnabled()) {
		enable();
		float attack = std::max(mAttack + float(mAttackJitter*Rand::randFloat(-1,1)), .001f);
		float decay = std::max(mDecay + float(mDecayJitter*Rand::randFloat(-1,1)), 0.001f);
		float sustain = ci::clamp(mSustain + float(mSustainJitter*Rand::randFloat(-1,1)), 0.0f, 1.0f);
		float hold = mHold + float(mSustainJitter*Rand::randFloat(-1, 1)) - (attack + decay);
		mAttackRamp =	mGainRef->getParam()->applyRamp(mVolume, attack);
		mDecayRamp = mGainRef->getParam()->appendRamp(sustain*mVolume,decay);
		if(hold>0.0f) mHoldRamp = mGainRef->getParam()->appendRamp(sustain*mVolume, hold);
		float release = std::max(mRelease + float(mReleaseJitter*Rand::randFloat(-1, 1)), .001f);
		mReleaseRamp =	mGainRef->getParam()->appendRamp(0, std::max(release, .001f));
	}

}

// Held voice with an ADSR envelope
void Instrument::gate( bool g ){

    mGate = g;
    //ci::app::console()<<"Gate "<<g<<"\n";
    if( g ) {
        enable();
		float attack = std::max(mAttack + float(mAttackJitter*Rand::randFloat(-1, 1)), .001f);
		float decay = std::max(mDecay + float(mDecayJitter*Rand::randFloat(-1, 1)), 0.001f);
		float sustain = ci::clamp(mSustain + float(mSustainJitter*Rand::randFloat(-1, 1)), 0.0f, 1.0f);
		float hold = mHold + float(mSustainJitter*Rand::randFloat(-1, 1)) - (attack + decay);
        mAttackRamp = mGainRef->getParam()->applyRamp(mVolume, std::max(attack, .001f));              // attack
        mDecayRamp = mGainRef->getParam()->appendRamp(sustain*mVolume, std::max(decay,0.001f));     // decay
        mReleaseRamp = mGainRef->getParam()->appendRamp(sustain*mVolume, INT_MAX ); // sustain
    } else {
		ci::app::console() << mName << " release ramp" << std::endl;
		float release = std::max(mRelease + float(mReleaseJitter*Rand::randFloat(-1, 1)), .001f);
        mReleaseRamp = mGainRef->getParam()->applyRamp( 0, std::max(release, .001f));
    }
    
}

void Instrument::choke( float v ){

	//ci::app::console() << "Choking voice " << getName() << std::endl;
    mReleaseRamp = mGainRef->getParam()->applyRamp( 0, v );
}

void Instrument::setPan( float v ){

    mPan = ci::clamp(v,0.0f,1.0f);
    mPanRef->setPos( mPan );

}

void Instrument::update(){
	//if( isEnabled() ) ci::app::console() << getName() << " " << mReleaseRamp->getTimeEnd() - ci::audio::master()->getNumProcessedSeconds()<<std::endl;
    if( isEnabled() && mReleaseRamp->isComplete()  ) disable();

}