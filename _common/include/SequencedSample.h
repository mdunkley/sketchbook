#pragma once

#include <stdio.h>
#include "cinder/app/App.h"
#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/Buffer.h"
#include "cinder/audio/Context.h"
#include "DistortionNode.h"
#include "cinder/Rand.h"
#include "PolySamplePlayer.hpp"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "LiveChannel.h"

using namespace ci;
using namespace std;

typedef std::shared_ptr<class SequencedSample> SequencedSampleRef;

class SequencedSample : public PolySamplePlayer {
public:

	SequencedSample();
	~SequencedSample();


	void setChannel(LiveChannelRef channel, int sceneId = 0);
	void setScene(LiveSceneDataRef& sceneData);
	void setScene(int sceneId);
	int  getScene() const { return mScene; }
	void setBufferInfo(BufferInfoRef info);
	void setPattern(PatternRef pat);
	void setPattern(int ptn);
	void triggerNote(NoteEvent& event);
	void fadeIn(float duration = .1f);
	void fadeOut(float duration = .1f, float delay=0.0f);

	void enableDistortion() { mDistortionRef->enable(); }
	void disableDistortion() { mDistortionRef->disable(); }
	
	void setDownsample(float const downsample) { enableDistortion(); mDownsample = downsample; }
	float getDownsample() const { return mDownsample; }
	

	void enableLowPass() { mLowPassRef->enable(); }
	void disableLowPass() { mLowPassRef->disable(); }
	void setLowPassFreq(float const freq) { enableLowPass(); mLowPassFreq = freq; }
	float getLowPassFreq() const { return mLowPassFreq; }
	void setLowPassRes(float const q) { enableLowPass(); mLowPassRes = q; }
	float getLowPassRes() const { return mLowPassRes; }


	void enableHighPass() { mHighPassRef->enable(); }
	void disableHighPass() { mHighPassRef->disable(); }
	void setHighPassFreq(float const freq) { enableLowPass(); mHighPassFreq = freq; }
	float getHighPassFreq() const { return mLowPassFreq; }
	void setHighPassRes(float const q) { enableLowPass(); mHighPassRes = q; }
	float getHighPassRes() const { return mHighPassRes; }

	bool isActive() const { return mActive; }

	void release();

	void resetParameters();

	SequencerRef& getSequencer() { return mSequencer;  }
	ci::audio::EventRef getFadeRamp() const { return mFadeRamp; }
	float	getFadeValue() const { return mVolumeRef->getValue(); }

	float			mDownsample = 0.0f;
	float			mLowPassFreq = 20000;
	float			mLowPassRes = 0.0f;
	float			mHighPassFreq = 0;
	float			mHighPassRes = 0.0f;

private:

	LiveChannelRef	mLiveChannel;
	BufferInfoRef	mBufferInfo;
	SequencerRef	mSequencer;
	SampleBankRef	mSampleBank;
	BufferRef		mSample;
	PatternRef		mPattern;
	bool			mActive;
	int				mScene;

	std::vector<PatternRef> mAllPatterns;
	signals::Connection mNoteConnection;
	
	int mCurrentPattern = 0;

	ci::audio::GainNodeRef	mVolumeRef;
	ci::audio::FilterLowPassNodeRef mLowPassRef;
	ci::audio::FilterHighPassNodeRef mHighPassRef;
	DistortionNodeRef mDistortionRef;

	ci::audio::EventRef mFadeRamp;

};

