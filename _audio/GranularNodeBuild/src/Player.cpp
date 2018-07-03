#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "SampleBank.h"
#include "Sequencer.hpp"
#include "GrainNode.h"
#include "CinderImGui.h"



using namespace ci;
using namespace ci::app;
using namespace std;

class PlayerApp : public App {
  public:
	void setup() override;
	void mouseUp(MouseEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void mouseMove(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void keyDown(KeyEvent event);
	void update() override;
	bool inspector();
	void draw() override;

	GrainNodeRef mPlayer;
	AudioManagerRef mAudioManager;
	SampleBankRef mRootBank;

	SequencerRef mClock;

	bool mShowMenu = true;
};

void PlayerApp::setup()
{
	ui::initialize();
	auto ctx = ci::audio::master();

	mRootBank = make_shared<SampleBank>();

	Rand::randomize();

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio");
	app::addAssetDirectory( commonAudioPath );

	mClock = std::make_shared<Sequencer>();
	mPlayer = ctx->makeNode( new GrainNode(ci::audio::Node::Format().channels(2)) );
	mPlayer >> ctx->getOutput();

	//mPlayer->setPanJitter(1);
	mPlayer->setPositionJitter(.1f);
	mPlayer->setTriggerSpeed(.05f);
	mPlayer->setTriggerSpeedJitter(.03f);
	//mPlayer->setIntervalJitter(12);
	mPlayer->setLength(1.0f);
	std::list<int> scale = { 0,5 };
	mPlayer->setScale(scale);
	mPlayer->enable();
	mPlayer->setVolume(.1);
	mPlayer->setVolumeJitter(.25);
	mPlayer->setPanJitter(.5);

	mRootBank->loadAssetDirectoryByName("test");
	ci::audio::BufferRef buffer = mRootBank->getRandomBuffer();
	mPlayer->setBuffer(buffer);


	mClock->start();
	ctx->enable();

}

void PlayerApp::mouseUp(MouseEvent event)
{
	mPlayer->gate(false);
}

void PlayerApp::mouseDown( MouseEvent event )
{
	mPlayer->gate(true);
}

void PlayerApp::mouseMove(MouseEvent event) {

	float relX = ci::clamp( event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1-(event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	mPlayer->setPosition(relX);
	mPlayer->setInterval(std::floor(72 * (relY - .5)));

}

void PlayerApp::mouseDrag(MouseEvent event) {

	float relX = ci::clamp(event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1 - (event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	mPlayer->setPosition(relX);
	mPlayer->setInterval(std::floor(72 * (relY - .5)));

}

void PlayerApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	switch (c) {

		case 'n':
			mPlayer->setBuffer(mRootBank->getNextBuffer());
			break;
	}
}

void PlayerApp::update()
{
	mClock->update();
	inspector();
	//console() << mPlayer->getNumActiveGrains() << std::endl;
}

bool PlayerApp::inspector()
{
	if (mShowMenu) {


		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		ui::SetWindowFontScale(getDisplay()->getWidth()/1920.0f);
		ui::DragFloat("Trigger Rate", &(mPlayer->mTriggerRate), .001, 0, 1);
		ui::DragFloat("Trigger Rate Jitter", &(mPlayer->mTriggerRateJitter), .001, 0, 1);
		ui::DragFloat("Rate", &(mPlayer->mRate), .001, -1, 1);
		ui::DragFloat("Rate Jitter", &(mPlayer->mRateJitter), .001, 0, 1);
		ui::DragFloat("Interval", &(mPlayer->mInterval),1, -36, 36);
		ui::DragFloat("Interval Jitter", &(mPlayer->mIntervalJitter),1, 0, 60);
		ui::DragFloat("Length", &(mPlayer->mLength),.001, 0, 5);
		ui::DragFloat("Length Jitter", &(mPlayer->mLengthJitter),.001, 0, 5);
		ui::DragFloat("Pan", &(mPlayer->mPan),.001, 0, 1);
		ui::DragFloat("Pan Jitter", &(mPlayer->mPanJitter),.001, 0,1);
		ui::DragFloat("Volume", &(mPlayer->mVolume), .001, 0, 5);
		ui::DragFloat("Volume Jitter", &(mPlayer->mVolumeJitter), .001, 0, 5);
		

	}
}

void PlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( PlayerApp, RendererGl )
