//
//  Recorder.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/23/18.
//

#include "Recorder.hpp"
#include "cinder/app/AppBase.h"

Recorder::Recorder(){

	mInput = audio::master()->createInputDeviceNode();
	mNodes.push_back(mInput);

	mRecordingGainNode = ci::audio::master()->makeNode(new audio::GainNode());
	mRecordingGainNode->setValue(1.0f);
	mNodes.push_back(mRecordingGainNode);

	mBufferRecorder = ci::audio::master()->makeNode(new audio::BufferRecorderNode(ci::audio::Node::Format().channels(2)));
	mBufferRecorder->setNumSeconds(60.0);
	mNodes.push_back(mBufferRecorder);

	if (!mInput) mInput >> mRecordingGainNode;
	mRecordingGainNode >> mBufferRecorder;

	disable();
}

Recorder::~Recorder(){
    
}

void Recorder::attachTo( Instrument* inst ){

    disable();
    mRecordingGainNode->disconnectAllInputs();
    inst->getBusOutput()>>mRecordingGainNode;
    enable();

}

void Recorder::attachTo( ci::audio::NodeRef n ){
    
    disable();
    mRecordingGainNode->disconnectAllInputs();
    n >> mRecordingGainNode;
    enable();
    
}

audio::BufferRef Recorder::getRecordedCopy(){
    return mBufferRecorder->getRecordedCopy();
}

void Recorder::toggleRecord() {
	record(!isRecording());
}

void Recorder::setChannels(int channels) {


}

void Recorder::record( bool r ){

    if( r ){
        enable();
        mBufferRecorder->start();
        //ci::app::console()<<"Starting record\n";

    } else {
        mBufferRecorder->stop();
        //ci::app::console()<<"Stopping record\n";
		mBufferRecorder->writeToFile("test.wav");
        disable();
    }

    mIsRecording = r;

}


