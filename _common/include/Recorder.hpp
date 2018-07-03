//
//  Recorder.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/23/18.
//

#ifndef Recorder_hpp
#define Recorder_hpp

#include <stdio.h>
#include "cinder/audio/SampleRecorderNode.h"
#include "cinder/audio/GainNode.h"
#include "Instrument.hpp"

class Recorder : public Instrument {
    
public:
    
    Recorder();
    ~Recorder();
    
    void attachTo( Instrument* inst );
    void attachTo( ci::audio::NodeRef n );
    void record( bool r );
    bool isRecording() const { return mIsRecording; }
    audio::BufferRef getRecordedCopy();

	void toggleRecord();

	void setChannels(int channels);

    
private:
    bool mIsRecording = false;
    audio::NodeRef mSourceNode;
    audio::BufferRecorderNodeRef mBufferRecorder;
    audio::GainNodeRef mRecordingGainNode;
    audio::InputDeviceNodeRef mInput;

};

#endif /* Recorder_hpp */
