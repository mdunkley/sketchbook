//
//  SampleVoice.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/15/18.
//

#include "SampleVoice.hpp"

SampleVoice::SampleVoice(){

	mBufferPlayer = audio::master()->makeNode(new PitchedBufferPlayerNode );
	mBufferPlayer >> getBusInput();
	mReleaseRamp = mGainRef->getParam()->applyRamp(0, INT_MAX);
	mNodes.push_back(mBufferPlayer);
	setLoopPosition(mLoopPosition);
	disable();

}

SampleVoice::~SampleVoice(){
    mBufferPlayer->disconnectAll();
}

void SampleVoice::setBuffer( ci::audio::BufferRef& buffer )
{
	mBuffer = buffer;
    mBufferLength = buffer->getNumFrames();
}

void SampleVoice::loadBuffer( string sourceFile )
{
    audio::SourceFileRef fileRef = audio::load( loadFile( sourceFile ), ci::audio::master()->getSampleRate() );
	audio::BufferRef buf = fileRef->loadBuffer();
	mBuffer = buf;
	mBufferLength = mBuffer->getNumFrames();
}

void SampleVoice::trigger() {

	mBufferPlayer->setBuffer(mBuffer);
	mBufferPlayer->setInterval(mInterval);
	mBufferPlayer->setRate(mRate);
	mBufferPlayer->start();
	mBufferPlayer->seek( getScaledSamplePosition(mPosition) );
	Instrument::trigger();

}

void SampleVoice::gate( bool g ){
    
    if( g ){
		mBufferPlayer->setBuffer(mBuffer);
		mBufferPlayer->setInterval(mInterval);
		mBufferPlayer->setRate(mRate);
        mBufferPlayer->start( );
        mBufferPlayer->seek(getScaledSamplePosition(mPosition));
    }
    Instrument::gate( g );

}

void SampleVoice::setLoopEnabled(bool l) {
	mLoopEnabled = l;
	mBufferPlayer->setLoopEnabled(l);
	setLoopPosition(getScaledSamplePosition(mLoopPosition));
}

void SampleVoice::setLoopPosition(double s) {
	mLoopPosition = s;
	mBufferPlayer->setLoopBegin(getScaledSamplePosition(s));
	mBufferPlayer->setLoopEnd(getScaledSamplePosition(1));
}

size_t SampleVoice::getScaledSamplePosition( double pos ) {

	// make sure start and end are sorted
	if (mSampleStart > mSampleEnd) { 
		double start = mSampleStart;
		mSampleStart = mSampleEnd;
		mSampleEnd = start;
	}

	double dist = mSampleEnd - mSampleStart;
	return size_t( mSampleStart+(pos*dist) * mBufferPlayer->getNumFrames() );
}

void SampleVoice::setSampleStart(double start) {
	mSampleStart = start;
	setLoopPosition(mLoopPosition);
}

void SampleVoice::setSampleEnd( double end ) {
	mSampleEnd = end;
	setLoopPosition(mLoopPosition);
}

double SampleVoice::getPosition() const {
    
	return mPosition;
}

size_t SampleVoice::getReadPosition() const {

	return mBufferPlayer->getReadPosition();
}

void SampleVoice::resetParameters() {

	mPosition = 0.0;
	mSampleStart = 0.0;
	mSampleEnd = 1.0;
	mLoopEnabled = false;
	mLoopPosition = 0.0;
	mRate = 1.0f;
	mInterval = 0.0f;

	Instrument::resetParameters();
}


