//
//  PolySamplePlayer.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/17/18.
//

#include "PolySamplePlayer.hpp"
#include "cinder/Log.h"


PolySamplePlayer::PolySamplePlayer(){
	mBufferInfo = make_shared<BufferInfo>();
	setNumSampleVoices(4);
	mGainRef->setValue(1);
	enable();
}


void PolySamplePlayer::setNumSampleVoices( int v  ){
    
    int dif = std::max(1,v)-int(mSampleVoices.size());
    if( dif>0 ){
        for(int i=0; i<dif; i++ ){
            auto s = SampleVoiceRef( new SampleVoice );
            s->setName( mName + "_voice_" + std::to_string(i) );
            s->disable();
			s->getBusOutput() >> getBusInput();
			//s->setLoopEnabled(false);
			s->setLoopPosition(0);
            mSampleVoices.emplace_back( s );
        }
    } else if( dif<0 ){
        for(int i=0; i<std::abs(dif); i++){
            mSampleVoices.pop_back();
        }
    }
    mNumSampleVoices = v;

}

void PolySamplePlayer::setName( string n ){

    mName = n;
    for( int i =0; i<mSampleVoices.size(); i++ ){
        mSampleVoices[i]->setName(mName + "_voice_" + std::to_string(i));
    }

}

void PolySamplePlayer::choke( float v )
{

	for (auto& voice : mSampleVoices) {
		if (voice->isEnabled())
			voice->choke();
	}
	//Instrument::choke( v );

}

void PolySamplePlayer::resetParameters()
{
 
	Instrument::resetParameters();
	mPosition = 0.0f;
	mPositionJitter = 0.0f;
	mDuration = .1f;
	mDurationJitter = 0.0f;
	mVolumeJitter = 0.0f;
	mPanJitter = 0.0f;
	mTriggerSpeed = .1f;
	mTriggerSpeedJitter = 0.0f;
	mTriggerSpeedJitterVal = 0;
	mProbability = 1.0f; // 0-1
	mSliceDivisions = 0;
	mSpreadProb = 1.0f;
	mReverseProb = 0.0f;

}

void PolySamplePlayer::gate( bool g ){
    mGate = g;
	if (g) trigger();
}

void PolySamplePlayer::update(){
    
    mHasTriggered = false;
    mTriggeredVoices.clear();
    
    // Update passthrough for voice self-monitoring
	for( auto& voice : mSampleVoices ) {
		voice->update();
    }

}

void PolySamplePlayer::disable()
{
	for( auto& voice : mSampleVoices ) {
		voice->disable();
	}
	Instrument::disable();
}

void PolySamplePlayer::setBuffer( ci::audio::BufferRef buffer )
{
	mBuffer = buffer;
	mBufferLength = mBuffer->getSize();
}

void PolySamplePlayer::setBuffer(ci::audio::BufferRef buffer, BufferInfoRef& info) {

	setBuffer(buffer);
	if (info) {
		if (info->slices.size() > 0) setSliceList(info->slices);
		if (info->noteMap.size() > 0) setSliceLookup(info->noteMap);
	}
}

void PolySamplePlayer::loadBuffer( string sourceFile ){

    audio::SourceFileRef fileRef = audio::load( loadFile( sourceFile ) );
	audio::BufferRef b = fileRef->loadBuffer();
    setBuffer( b );

}

void PolySamplePlayer::trigger(){

    bool spawn = false;
	if( mBuffer ) {
		for (auto& voice : mSampleVoices) {
			if (!voice->isEnabled()) {
				triggerVoice(voice);
				spawn = true;
				break;
			}
		}
    }

    mHasTriggered=spawn;
}

void PolySamplePlayer::triggerVoice( const SampleVoiceRef& voice ){

	if (mSlices.size() > 0) setToSlicePosition(mCurrentSlice);

	int dirMult = (mReverseProb > Rand::randFloat()) ? -1 : 1;
	voice->setRate(mRate * dirMult );
	voice->setInterval(mInterval);
	voice->setVolume(std::max(mVolume+mVolumeJitter*Rand::randFloat(),0.0f));
    voice->setAttack(mAttack);
	voice->setAttackJitter(mAttackJitter);
	voice->setSustain(mSustain);
	voice->setSustainJitter(mSustainJitter);
	voice->setHold(mHold);
	voice->setHoldJitter(mHoldJitter);
    voice->setDecay(mDecay);
	voice->setDecayJitter(mDecayJitter);
	voice->setRelease(mRelease);
	voice->setReleaseJitter(mReleaseJitter);
    voice->setPan(ci::clamp( mPan + mPanJitter*(Rand::randFloat(-1,1)),0.0f,1.0f));
    voice->setPosition( ci::clamp(mPosition + mPositionJitter * Rand::randFloat(), 0.0f, 1.0f) );
	voice->setBuffer(mBuffer);
    voice->trigger();

    mTriggeredVoices.push_back(voice);
}

void PolySamplePlayer::setSliceDivisions(int div = 32) {
	mSliceDivisions = std::max(0, div);
	if (div > 0) {
		mSlices.clear();
		float section = 1.0 / div;
		for (int i = 0; i < div; i++) {
			mSlices.push_back(section*i);
		}
		mSliceLookup.clear();
	}
}

void PolySamplePlayer::setToSlicePosition(int slice) {

	if (mSlices.size() > 0) {
		
		int cuedSlice = slice;

		if (mSliceSpread != 0.0f  && mSpreadProb>Rand::randFloat() ) {

			float spreadVal = ci::clamp(mSliceSpread, 0.0f, 1.0f) * mSlices.size() * Rand::randFloat(-1.0f,1.0f);
			if (spreadVal >= 0.0f) spreadVal = std::floor(spreadVal);
			else spreadVal = -1 * std::floor(-spreadVal);
			bool hasLookup = mLookupKeys.size() > 0;
			if(hasLookup) {
				auto search = std::find(mLookupKeys.begin(), mLookupKeys.end(), cuedSlice);
				if (search != mLookupKeys.end()) {
					int offset = int(spreadVal);
					int pos = search - mLookupKeys.begin();
					int lookup = ci::clamp(pos+offset, 0, int(mLookupKeys.size()-1));
					cuedSlice = mLookupKeys.at(lookup);
				}
			} else if (!hasLookup) {
				cuedSlice = ci::clamp(int(cuedSlice + spreadVal), 0, int(mSlices.size() - 1));
			}
		}

		if (mSliceLookup.size()>0) {
			auto val = mSliceLookup.find(cuedSlice);
			if (val != mSliceLookup.end()) cuedSlice = val->second;
		}
		else {
			cuedSlice = ci::clamp(cuedSlice, 0, int(mSlices.size() - 1));
		}

		mPosition = mSlices.at(cuedSlice);
	}
}

void PolySamplePlayer::sliceWalk() {
	if (Rand::randFloat() > .5) sliceIncrement();
	else sliceDecrement();
}

void PolySamplePlayer::setSliceList(std::vector<double>& sliceList) { 
	mSlices = sliceList; 
	mSliceDivisions = 0; 
}

void PolySamplePlayer::setSlice(int slice ) {
	mCurrentSlice = slice;
}

void PolySamplePlayer::sliceIncrement() {
	setSlice(mCurrentSlice+1);
}

void PolySamplePlayer::sliceDecrement() {
	setSlice(mCurrentSlice - 1);
}

size_t PolySamplePlayer::getNumSlices() {
	return mSlices.size(); 
}

void PolySamplePlayer::setSliceLookup(std::map<int, int>& lookup) {

	if (lookup.size()) {
		mSliceLookup = lookup;
		mLookupKeys.clear();
		for (auto& key : lookup) {
			mLookupKeys.push_back(key.first);
		}
		std::sort(mLookupKeys.begin(), mLookupKeys.end());
	}

}