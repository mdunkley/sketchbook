#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "AudioManager.hpp"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "SequencedSample.h"
#include "NoteSequencer.h"
#include <iostream>
#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PlayerApp : public App {

  public:

	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;

	void setProbability(MouseEvent event);
	bool inspector();
	void randomizeAll();
	void randomizeProbabilities();
	void setAllProbability( float value );
	void randomizeSpread();
	void setAllSpread( float value );

	AudioManagerRef mAudioManager;

	std::vector<SequencedSampleRef> mPlayers;
	PolySamplePlayerRef mTest;
	SampleBankRef		mTestBank;
	SequencerRef mClock;
	vec2 mRelPos;

	std::set<int> mAllNotes;
	std::map<int, int>								mNoteLookup;
	ci::audio::BufferRef							mBuffer;
	std::map<std::string, SampleBankRef>			mAllBanks;
	std::map<ci::audio::BufferRef, SampleBankRef>	mAllSampleInfo;

	std::vector<float> mProbabilities;
	
	SampleBankRef mRootBank;
	BufferInfoRef mInfo;

	int mBPM = 140;
	float mClockJitter = 0.0f;

	bool mShowMenu = true;
	float mRmsMult = 15;

	float mAllSpread = 0;
	float mAllProb = 0;

};

void PlayerApp::setup()
{
	ui::initialize();

	mRelPos = vec2(0, 0);
	mRootBank = make_shared<SampleBank>();

	Rand::randomize();

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio" );
	app::addAssetDirectory( commonAudioPath );
	mAudioManager = std::make_shared<AudioManager>();
	mClock = std::make_shared<Sequencer>();
	mTest = mAudioManager->getPolySamplePlayer(true);

	mRootBank->loadAssetDirectoryByName("hectare");
	mRootBank->printBufferNames();

	//mRootBank->loadAssetDirectory("beauford");
	//ci::app::console() << "SIZE " << mRootBank->getSize() << std::endl;
	for (auto& buf : mRootBank->getBuffersWithInfo()) {
		SequencedSampleRef s = mAudioManager->getSequencedSample();
		mClock->clock( s->getSequencer() );
		s->setBufferInfo(buf);
		mPlayers.push_back(s);
		mProbabilities.push_back(1.0);
	}	

	//ci::app::console() << "FOUND " << mRootBank->getSize() << std::endl;
	
	mClock->setBPM(mBPM,16);
	mClock->start();
	//mClock->setRateJitter(.0254);
}


void PlayerApp::mouseDown( MouseEvent event )
{
	setProbability(event);
}

void PlayerApp::mouseDrag(MouseEvent event) {
	setProbability(event);
}

void PlayerApp::setProbability(MouseEvent event) {

	float relX = ci::clamp(float(event.getX()) / app::getWindowWidth(), 0.0f, .999f);
	float relY = ci::clamp(float(event.getY()) / app::getWindowHeight(), 0.0f, .999f);
	float inst = relX * mPlayers.size();
	float frac = inst - std::floor(inst);
	if (frac < .5) {
		mPlayers.at(std::floor(inst))->getSequencer()->setProbability(1 - relY);
	}
	else {
		mPlayers.at(std::floor(inst))->setSliceSpread(1 - relY);
	}
	mRelPos = vec2(relX, relY);

}

void PlayerApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	/*
	if (c == '1') {
		for (auto& p : mPlayers) {
			p->setPattern(0);
		}
	}
	else if (c == '2') {
		for (auto& p : mPlayers) {
			p->setPattern(1);
		}
	}
	else if (c == '3') {
		for (auto& p : mPlayers) {
			p->setPattern(2);
		}
	}
	else if (c == '4') {
		for (auto& p : mPlayers) {
			p->setPattern(3);
		}
	}
	*/
	if (event.getCode() == event.KEY_SPACE) {
		mShowMenu = !mShowMenu;
		//randomize();
	}

}

bool PlayerApp::inspector()
{

	if (mShowMenu) {
		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		if (ui::DragInt("Beats Per Minute", &mBPM, 1, 10, 250)) {
			mClock->setBPM(mBPM);
		}
		ui::DragFloat("RMS Mult", &mRmsMult, .1, 0, 20);
		if (ui::DragFloat("Global Probability", &mAllProb, .01, 0, 1)) {
			setAllProbability(mAllProb);
		}
		if (ui::Button("Randomize Probability")) { randomizeProbabilities(); }
		if (ui::DragFloat("Global Spread", &mAllSpread, .01, 0, 1)) {
			setAllSpread(mAllSpread);
		}
		if (ui::Button("Randomize Slice Spread")) { randomizeSpread(); }
		if (ui::Button("Randomize All")) { randomizeAll(); }


		
		int index = 0;
		for (const auto& player : mPlayers ) {
			ui::ScopedId pushId(index);
			std::string name = std::to_string(index) + ":" + player->getName();

			if (ui::CollapsingHeader(name.c_str())) {

				ui::DragInt("Clock Division", &(player->getSequencer()->mClockDivision), 1, 1, 32);
				ui::DragFloat("Probability", &(player->getSequencer()->mProbability), 0.01, 0, 1);
				ui::DragInt("Beat Offset", &(player->getSequencer()->mBeatOffset), 1, 0, 64);
				ui::DragFloat("Slice Spread", &(player->mSliceSpread), .01, 0, 1);
				ui::DragFloat("Volume", &(player->mVolume), .01, 0, 1);
				ui::DragFloat("Volume Jitter", &(player->mVolumeJitter), .01, 0, 1);
				ui::DragFloat("Pan", &(player->mPan), .01, 0, 1);
				ui::DragFloat("Pan Jitter", &(player->mPanJitter), .01, 0, 1);
				ui::DragFloat("Attack", &(player->mAttack), .01, 0, 1);
				ui::DragFloat("Attack Jitter", &(player->mAttackJitter), .01, 0, 1);
				ui::DragFloat("Decay", &(player->mDecay), .01, 0, 1);
				ui::DragFloat("Decay Jitter", &(player->mDecayJitter), .01, 0, 1);
				ui::DragFloat("Hold", &(player->mHold), .001, 0, 1);
				ui::DragFloat("Hold Jitter", &(player->mHoldJitter), .001, 0, 1);
				ui::DragFloat("Sustain", &(player->mSustain), .01, 0, 1);
				ui::DragFloat("Sustain Jitter", &(player->mSustainJitter), .01, 0, 1);
				ui::DragFloat("Release", &(player->mRelease), .01, 0, 1);
				ui::DragFloat("Release Jitter", &(player->mReleaseJitter), .01, 0, 1);
				ui::DragFloat("Low Pass Freq", &(player->mLowPassFreq), 10, 0, 20000);
				ui::DragFloat("High Pass Freq", &(player->mHighPassFreq), 10, 0, 20000);
	
				ui::NewLine();
			}
			
			index++;
		}
		



	}
	return false;
}
void PlayerApp::update()
{
	mClock->update();
	mAudioManager->update();

	inspector();
}

void PlayerApp::randomizeAll() {
	randomizeProbabilities();
	randomizeSpread();
}

void PlayerApp::randomizeProbabilities() {
	for (auto& player : mPlayers) {
		player->getSequencer()->setProbability(Rand::randFloat());
	}
}

void PlayerApp::setAllProbability( float value ) {
	for (auto& player : mPlayers) {
		player->getSequencer()->setProbability(value);
	}
}

void PlayerApp::randomizeSpread() {
	for (auto& player : mPlayers) {
		player->setSliceSpread(Rand::randFloat());
	}
}

void PlayerApp::setAllSpread(float value) {
	for (auto& player : mPlayers) {
		player->setSliceSpread(value);
	}
}

void PlayerApp::draw()
{

	//inspector();
	gl::clear( Color( 0, 0, 0 ) ); 
	
	int instruments = mPlayers.size();
	vec2 windowSize = app::getWindowSize();
	float width = windowSize.x / instruments;
	int count = 0;
	for (auto& p : mPlayers) {
		float rms = ci::clamp( p->getRms()*mRmsMult,0.0f,1.0f);
		float prob = p->getSequencer()->getProbability();
		float spread = p->getSliceSpread();
		gl::color( prob, rms, rms, 1);
		gl::drawSolidRect(Rectf(
			width*count+5,
			windowSize.y,
			width*(count+.5),
			windowSize.y-(windowSize.y*prob)));
		gl::color(rms, spread, rms, 1);
		gl::drawSolidRect(Rectf(
			width*(count + .5),
			windowSize.y,
			width*(count + 1) - 5,
			windowSize.y - (windowSize.y*spread)));
		count++;
	}

}

CINDER_APP( PlayerApp, RendererGl )
