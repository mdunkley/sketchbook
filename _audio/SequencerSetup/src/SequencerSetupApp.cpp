#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "PolySamplePlayer.hpp"
#include "WavetableVoice.hpp"
#include "SampleBank.h"
#include "DistortionNode.h"
#include "Sequencer.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

// x position adds offset to note
// 1 2 3 4 selects quantizer scale

class SequencerSetupApp : public App {

public:

	void setup() override;
	void keyDown(KeyEvent event) override;
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

	void triggerSynthVoice(StepInfo info);
	void triggerBassVoice(StepInfo info);
	void setNoteOffset(StepInfo info);

	
	std::vector<int> mScale;
	std::vector<int> mScale0;
	std::vector<int> mScale1;
	std::vector<int> mScale2;
	std::vector<int> mScale3;

	SampleBankRef mBuffers;
	AudioManagerRef mAudioManager;
	PolySamplePlayerRef mPlayer;
	DistortionNodeRef mDistortion;

	SequencerRef mClock;
	SequencerRef mArpSeq;
	SequencerRef mBassSeq;
	SequencerRef mOffsetSeq;

	float mMouseX = 0.0;
	float mMouseY = 0.0;

	int mOffset = 0;
	int mNoteOffset = 0;


};

void SequencerSetupApp::setup()
{

	mScale0 = { 0,2,4,5,7,9,11};
	mScale1 = { 1,3,6,8,10 };
	mScale2 = { 0,3,5,8,10 };
	mScale3 = { 0,2,4,6,11 };

	mScale = mScale3;
	
	// Load Buffers
	mBuffers = make_shared<SampleBank>();
	mBuffers->loadAssetDirectory("bank_01");

	// Load audio manager
	mAudioManager = std::make_shared<AudioManager>();
	ci::audio::NodeRef out = audio::master()->getOutput();

	mAudioManager->getWavetableVoices(16);

	mClock = std::make_shared<Sequencer>();
	mArpSeq = std::make_shared<Sequencer>();
	mBassSeq = std::make_shared<Sequencer>();
	mOffsetSeq = std::make_shared<Sequencer>();
	
	std::vector<double> offsetSeq = { 0, 3, 5, 7, 0, 2, 4, 12 };
	mOffsetSeq->addModulationSequence(offsetSeq);
	mOffsetSeq->setClockDivision(64);
	mOffsetSeq->getClockSignal().connect( [&](StepInfo s) { setNoteOffset(s); });

	std::vector<double> arpSeq = { 64, 66, 70, 72, 64, 74 };
	std::vector<double> arpModSeq = { .1,.05,.05,.08,.015 };
	std::vector<int> arpTriggerSeq = { 1,1,0,1,1,0,1 };
	mArpSeq->addModulationSequence( arpSeq );
	mArpSeq->addModulationSequence(arpModSeq);
	mArpSeq->setTriggerSequence(arpTriggerSeq);
	mArpSeq->setClockDivision(4);
	mArpSeq->getTriggerSignal().connect( [&](StepInfo s) { triggerSynthVoice(s); } );

	std::vector<double> bassSeqSrc = { 52, 55, 59, 68, 65 };
	std::vector<double> bassSeq;
	for (double b : bassSeqSrc)  for (int i = 0; i < 4; i++) bassSeq.push_back(b);
	mBassSeq->addModulationSequence(bassSeq);
	mBassSeq->addModulationSequence(arpModSeq);
	mBassSeq->setClockDivision(32);
	mBassSeq->getClockSignal().connect( [&](StepInfo s) { triggerBassVoice(s); } );

	mClock->setBPM(100, 32);
	mClock->start();
	mClock->clock(mOffsetSeq);
	mClock->clock(mBassSeq);
	mClock->clock(mArpSeq);
}

void SequencerSetupApp::mouseMove(MouseEvent event)
{
	mOffset = floor((float(event.getX()) / app::getWindowWidth() - .5) * 12);
	//ci::app::console() << mOffset << std::endl;
	//mDistortion->setSampleRateMult(float(event.getY()) / app::getWindowHeight());
	//mDistortion->setBitRateMult(float(event.getY()) / app::getWindowHeight());
	//mDistortion->setMultiply(float(event.getY()) / app::getWindowHeight()*100);
}

void SequencerSetupApp::mouseDown(MouseEvent event)
{
}

void SequencerSetupApp::triggerBassVoice(StepInfo info) {
	if (info.values.size() > 0) {
		WavetableVoiceRef v = mAudioManager->getWavetableVoice();
		v->setScale(mScale);
		v->setInUse(true);
		v->setFreqMidi(mOffset + mNoteOffset + info.values.at(0) - 15);
		v->setAttack(.1);
		v->setRelease(.5);
		v->setVolume(.05);
		v->trigger();
	}
}

void SequencerSetupApp::triggerSynthVoice(StepInfo info) {
	if (info.values.size() > 1) {
		//ci::app::console() << "I'm playing a voice i guess" << std::endl;
		WavetableVoiceRef v = mAudioManager->getWavetableVoice();
		v->setScale(mScale);
		v->setInUse(true);
		v->setFreqMidi(mNoteOffset + mOffset + info.values.at(0));
		v->setAttack(.1);
		v->setRelease(info.values.at(1));
		v->setVolume(.05);
		v->trigger();
	}
}

void SequencerSetupApp::setNoteOffset(StepInfo info) {
	if (info.values.size() > 0) {
		mNoteOffset = info.values.at(0);
	}
}

void SequencerSetupApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	if (c == '1')  mScale = mScale0;
	else if(c=='2') mScale = mScale1;
	else if(c=='3') mScale = mScale2;
	else if(c=='4') mScale = mScale3;

}

void SequencerSetupApp::update()
{
	mAudioManager->update();
	mClock->update();
}

void SequencerSetupApp::draw()
{
	mAudioManager->update();
	gl::clear(Color(0, 0, 0));
}

CINDER_APP(SequencerSetupApp, RendererGl)
