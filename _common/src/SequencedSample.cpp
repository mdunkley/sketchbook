#include "SequencedSample.h"
#include "cinder/Log.h"

SequencedSample::SequencedSample() {

	mSequencer =	make_shared<Sequencer>();
	mSampleBank =	make_shared<SampleBank>();
	mSample =		make_shared<Buffer>();
	mPattern =		make_shared<Pattern>();

	mSequencer->getNoteSignal().connect( [this](NoteEvent m) { triggerNote(m); } );
	
	mVolumeRef = ci::audio::master()->makeNode(new ci::audio::GainNode);
	mHighPassRef = ci::audio::master()->makeNode(new ci::audio::FilterHighPassNode);
	mLowPassRef = ci::audio::master()->makeNode(new ci::audio::FilterLowPassNode);
	mDistortionRef = ci::audio::master()->makeNode(new DistortionNode);
	mMonitorRef->disconnectAllInputs();
	getBusOutput() >> mDistortionRef >> mLowPassRef >> mHighPassRef >> mVolumeRef;
	mHighPassRef >> mMonitorRef;
	mFxNodes.push_back(mDistortionRef);
	mFxNodes.push_back(mLowPassRef);
	mFxNodes.push_back(mHighPassRef);
	mNodes.push_back(mVolumeRef);
	setBusOutput(mVolumeRef);

	mLowPassRef->setCutoffFreq(20000);
	mHighPassRef->setCutoffFreq(0);

	mFadeRamp = mVolumeRef->getParam()->applyRamp(1, INT_MAX);
}

SequencedSample::~SequencedSample() {

}

void SequencedSample::setChannel(LiveChannelRef channel, int sceneId)
{
	mLiveChannel = channel;
	setScene( mLiveChannel->getScene(sceneId) );
}

void SequencedSample::setScene(LiveSceneDataRef& sceneData)
{
	if (sceneData->active) {
		clearSliceLookup();
		setBuffer(sceneData->buffer);
		setPattern( sceneData->divPattern ) ;
		setSliceDivisions(sceneData->divisions);
		getSequencer()->start();
		mActive = true;
	}
	else {
		getSequencer()->stop();
		mActive = false;
	}
}

void SequencedSample::setScene(int sceneId)
{
	setScene(mLiveChannel->getScene(sceneId));
}

void SequencedSample::setBufferInfo(BufferInfoRef info) {
	
	mBufferInfo = info;

	if (mSliceDivisions > 0) {
		setBuffer(info->buffer);
		float noteLength = mBuffer->getNumFrames() / mSliceDivisions;
		PatternRef pat = make_shared<Pattern>();
		for (int i = 0; i < mSliceDivisions; i++) {
			NoteEvent noteEvent{ i, noteLength,	1.0f };
			pat->addNote( i * 16 * 16, noteEvent );
		}
		clearSliceLookup();

	}
	else {

		mAllPatterns = info->patterns;
		setPattern(mCurrentPattern);
		setBuffer(info->buffer);
		setSliceList(info->slices);
		if( (info->noteMap).size() ) setSliceLookup(info->noteMap);
	}
}

void SequencedSample::setPattern(PatternRef pat) {
	mPattern = pat;
	mSequencer->setNoteSequence(pat);
	mSequencer->setLength(pat->getLength());
}

void SequencedSample::setPattern(int ptn) {

	mCurrentPattern = std::max(0,ptn);
	if (ptn >= mAllPatterns.size()) {

		mSequencer->clearNoteSequence();
	}
	else {

		mSequencer->setNoteSequence(mAllPatterns.at(mCurrentPattern));
		mSequencer->setLength(mAllPatterns.at(mCurrentPattern)->getLength());
	}

}


void SequencedSample::triggerNote(NoteEvent& event) {

	setSlice(event.note);
	setHold(event.duration);

	mDistortionRef->setSampleRateMult(mDownsample);
	mLowPassRef->setCutoffFreq(	mLowPassFreq	);
	mLowPassRef->setResonance(mLowPassRes);
	mHighPassRef->setCutoffFreq(	mHighPassFreq	);
	mHighPassRef->setResonance(mHighPassRes);

	PolySamplePlayer::trigger();

}

void SequencedSample::release() {
	resetParameters();
	mAllPatterns.clear();
	mSequencer->clearNoteSequence();
	mSequencer->stop();
	mInUse = false;
	mActive = false;
}

void SequencedSample::resetParameters() {

	PolySamplePlayer::resetParameters();
	mPan = .5;
	mDownsample = 0;
	mLowPassFreq = 20000.0f;
	mLowPassRes = 0.0f;
	mHighPassFreq = 0;
	mHighPassRes = 0.0f;

}

void SequencedSample::fadeIn(float duration)
{
	mFadeRamp = mVolumeRef->getParam()->applyRamp( 1.0f, duration );
}

void SequencedSample::fadeOut(float duration, float delay )
{
	audio::Param::Options options{};
	if( delay > 0 ) {
		options.delay( delay );
	}
	mFadeRamp = mVolumeRef->getParam()->applyRamp( 0.0f, duration, options );
}