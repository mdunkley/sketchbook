#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "AudioManager.hpp"
#include "SampleBank.h"
#include "Sequencer.hpp"
#include "GranularPlayer.hpp"

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
	void update() override;
	void draw() override;

	GranularPlayerRef mPlayer;
	AudioManagerRef mAudioManager;
	SampleBankRef mRootBank;

	SequencerRef mClock;
};

void GranularPlayerApp::setup()
{


	mRootBank = make_shared<SampleBank>();

	Rand::randomize();

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio");
	app::addAssetDirectory(commonAudioPath);
	mAudioManager = std::make_shared<AudioManager>();
	mClock = std::make_shared<Sequencer>();
	mPlayer = mAudioManager->getGranularPlayer();
	mRootBank->loadAssetDirectoryByName("test");
	mPlayer->setBuffer(mRootBank->getRandomBuffer());
	mClock->start();

	mPlayer->setPanJitter(1);
	mPlayer->setPositionJitter(.005);
	mPlayer->setTriggerSpeed(.01);
	mPlayer->setTriggerSpeedJitter(.01);

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
}

void GranularPlayerApp::mouseDrag(MouseEvent event) {
	float relX = ci::clamp(event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1 - (event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	mPlayer->setPosition(relX);
	mPlayer->setInterval(std::floor(48 * (relY - .5)));
}

void GranularPlayerApp::update()
{
	mClock->update();
	mAudioManager->update();
}

void GranularPlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( GranularPlayerApp, RendererGl )
