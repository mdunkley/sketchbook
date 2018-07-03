#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "SampleBank.h"
#include "Sequencer.hpp"
#include "GranularPlayer.hpp"
#include "GrainNode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GranularPlayerApp : public App {
  public:
	void setup() override;
	void mouseUp(MouseEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void mouseMove(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void keyDown(KeyEvent event);
	void update() override;
	void draw() override;

	GrainNodeRef mPlayer;
	AudioManagerRef mAudioManager;
	SampleBankRef mRootBank;

	SequencerRef mClock;
};

void GranularPlayerApp::setup()
{
	auto ctx = ci::audio::master();

	mRootBank = make_shared<SampleBank>();

	Rand::randomize();

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio");
	app::addAssetDirectory(commonAudioPath);

	mClock = std::make_shared<Sequencer>();
	mPlayer = ctx->makeNode(new GrainNode(ci::audio::Node::Format().channels(2)));
	mPlayer >> ctx->getOutput();



	//mPlayer->setPanJitter(1);
	mPlayer->setPositionJitter(.1f);
	mPlayer->setTriggerSpeed(.02f);
	mPlayer->setTriggerSpeedJitter(.03f);
	mPlayer->setIntervalJitter(12);
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

void GranularPlayerApp::mouseUp(MouseEvent event)
{
	mPlayer->gate(false);
}

void GranularPlayerApp::mouseDown( MouseEvent event )
{
	mPlayer->gate(true);
}

void GranularPlayerApp::mouseMove(MouseEvent event) {

	float relX = ci::clamp( event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1-(event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	mPlayer->setPosition(relX);
	mPlayer->setInterval(std::floor(72 * (relY - .5)));

}

void GranularPlayerApp::mouseDrag(MouseEvent event) {

	float relX = ci::clamp(event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1 - (event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	mPlayer->setPosition(relX);
	mPlayer->setInterval(std::floor(72 * (relY - .5)));

}

void GranularPlayerApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	switch (c) {

		case 'n':
			mPlayer->setBuffer(mRootBank->getNextBuffer());
			break;
	}
}

void GranularPlayerApp::update()
{
	mClock->update();
	//console() << mPlayer->getNumActiveGrains() << std::endl;
}

void GranularPlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( GranularPlayerApp, RendererGl )
