#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "PolySamplePlayer.hpp"
#include "SampleBank.h"
#include "DistortionNode.h"
#include "Sequencer.hpp"
#include "MoogFilterNode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class fxBuildApp : public App {

  public:

	void setup() override;
	void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;

	SampleBankRef mBuffers;
	AudioManagerRef mAudioManager;
	PolySamplePlayerRef mPlayer;
	DistortionNodeRef mDistortion;
	MoogFilterNodeRef mMoogFilter;
	SequencerRef mClock;

};

void fxBuildApp::setup()
{

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio");
	app::addAssetDirectory(commonAudioPath);

	// Load Buffers
	mBuffers = make_shared<SampleBank>();
	mBuffers->loadAssetDirectoryByName("bank_01");

	// Load audio manager
	mAudioManager = std::make_shared<AudioManager>();
	ci::audio::NodeRef out = audio::master()->getOutput();
	out->disconnectAllInputs();

	mDistortion = audio::master()->makeNode(new DistortionNode);
	mMoogFilter = audio::master()->makeNode(new MoogFilterNode);

	mPlayer = mAudioManager->getPolySamplePlayer();
	mPlayer->setBuffer( mBuffers->getRandomBuffer() );

	mPlayer->getBusOutput() >> mDistortion >> mMoogFilter >> out;

	mClock = std::make_shared<Sequencer>();
	mClock->setBPM(100, 4);
	mClock->getClockSignal().connect([&](StepInfo m) { mPlayer->setAttack(.2);  mPlayer->trigger(); });
	mClock->start();
}

void fxBuildApp::mouseMove( MouseEvent event )
{

	float relx = clamp(float(event.getX()) / app::getWindowWidth(),0.0f,1.0f);
	float rely = clamp(float(event.getY()) / app::getWindowWidth(),0.0f,1.0f);

	mDistortion->setSampleRateMult(relx);
	mMoogFilter->setFrequencey(rely);
	mMoogFilter->setResonance(relx*4);
	//mDistortion->setBitRateMult(rely);
}

void fxBuildApp::update()
{
	mClock->update();
}

void fxBuildApp::draw()
{
	mAudioManager->update();
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( fxBuildApp, RendererGl )
