#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "PolySamplePlayer.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;


class SliceSequenceApp : public App {

  public:

	void prepareSettings(Settings *settings);
	void setup() override;
	void keyDown(KeyEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;




	AudioManagerRef mAudioManager;

	SampleBankRef mSampleBank;
	SampleBankRef mKeySequences;

	SequencerRef mClock;
	SequencerRef mChainClock;

	void triggerVoice(StepInfo s, int channel=0);

};

void SliceSequenceApp::prepareSettings(Settings *settings)
{
	settings->setHighDensityDisplayEnabled(true);
}

void SliceSequenceApp::setup()
{
	// Make manager, banks, and sequencers
	mAudioManager = std::make_shared<AudioManager>();
	mSampleBank = std::make_shared<SampleBank>();
	mKeySequences = std::make_shared <SampleBank>();
	mClock = std::make_shared<Sequencer>();
	mChainClock = std::make_shared<Sequencer>();

	// Load samples
	mSampleBank->loadAssetDirectory("bank_00");

	// Setup core clock
	mClock->setBPM(120, 8);
	mClock->start();
	mChainClock->setClockDivision(1);
	mClock->clock(mChainClock);

	// Randomize the randomizer
	Rand::randomize();

	mAudioManager->getPolySamplePlayers(10);

	mChainClock->getClockSignal().connect([this](StepInfo s){ triggerVoice(s,0); });
	
}

void SliceSequenceApp::triggerVoice(StepInfo s, int channel) {

	PolySamplePlayerRef p = mAudioManager->getPolySamplePlayer( false );
	p->setPosition(Rand::randFloat());
	p->setBuffer(mSampleBank->getBuffer(0));
	p->setHold(.5);
	p->trigger();
}

void SliceSequenceApp::keyDown(KeyEvent event) {
	char c = event.getChar();
}
void SliceSequenceApp::mouseDown( MouseEvent event )
{
}

void SliceSequenceApp::update()
{
	mClock->update();
	mAudioManager->update();

	//console() << mAudioManager->getAllPolySamplePlayers().size() << std::endl;
}

void SliceSequenceApp::draw()
{
	gl::clear(Color(0, 0, 0));


}


CINDER_APP( SliceSequenceApp, RendererGl )
