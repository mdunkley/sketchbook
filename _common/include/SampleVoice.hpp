//
//  SampleVoice.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/15/18.
//

#ifndef SampleVoice_hpp
#define SampleVoice_hpp

#include <stdio.h>
#include "Instrument.hpp"
#include "cinder/audio/audio.h"
#include "PitchedBufferPlayerNode.hpp"
#include "cinder/audio/FilterNode.h"

using namespace ci;
using namespace std;

typedef std::shared_ptr<class SampleVoice>	SampleVoiceRef;

class SampleVoice : public Instrument {
    
public:

    SampleVoice();

    virtual ~SampleVoice();

    
    void setBuffer( ci::audio::BufferRef& buffer );
    void loadBuffer( string sourceFile );
	ci::audio::BufferRef getBuffer() { return mBuffer; }

	void setSampleStart(double start);
	double getSampleStart() const { return mSampleStart; }
	void   setSampleEnd(double end);
	double getSampleEnd() const { return mSampleEnd; }

	bool isLoopEnabled() const { return mLoopEnabled; }
	void setLoopEnabled(bool l);
	void setLoopPosition(double s );
	double getLoopPosition() { return mLoopPosition; }
    
    double getPosition() const;
	void setPosition(double f) { mPosition = f; }
	size_t getReadPosition() const;

	void resetParameters();

    void gate( bool g ) override;
    void trigger() override;


	// Pitch related members
	float getInterval() const { return mInterval; }
	void setInterval(float interval) { mInterval = interval; }
	float mInterval = 0;

	float getRate() const { return mRate; }
	void setRate(float rate) { mRate = rate; }
	float mRate = 1.0f;
    
private:
    
	size_t getScaledSamplePosition( double pos );

    PitchedBufferPlayerNodeRef  mBufferPlayer;
	audio::BufferRef			mBuffer;
    size_t mBufferLength;
    
    double	mPosition = 0.0;
	double	mSampleStart = 0.0;
	double	mSampleEnd = 1.0;
	bool	mLoopEnabled = false;
	double	mLoopPosition = 0.0;

};

#endif /* SampleVoice_hpp */

