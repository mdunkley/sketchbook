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
#include "LiveChannel.h"
#include "AudioDrawUtils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PlayerApp : public App {

  public:

	void setup() override;
	void clearChannels();
	void loadChannels();
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
	void randomizeScenes();
	void setAllSpread( float value );
	void setAllSpreadProb(float value);
	void sceneOffset(int count);
	int  wrap(int kX, int const kLowerBound, int const kUpperBound);

	void setBeauford();
	void setHectare();

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

	LiveChannelBankRef mBeaufordBank;
	LiveChannelBankRef mHectareBank;
	LiveChannelBankRef mChannelBank;

	std::vector<float> mProbabilities;
	std::vector<string> mChannelNames;
	std::vector<int> mVoiceChannels;
	std::vector<int> mScenes;
	
	SampleBankRef mRootBank;
	BufferInfoRef mInfo;

	int mBPM = 140;
	float mClockJitter = 0.0f;
	int mScene = 0;

	bool mShowMenu = true;
	float mRmsMult = 35;

	float mAllSpread = 0;
	float mAllSpreadProb = 0;
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

	mBeaufordBank = make_shared<LiveChannelBank>();
	mBeaufordBank->loadChannelsFromAssetsDirectory("beauford/splits",32);

	mHectareBank = make_shared<LiveChannelBank>();
	mHectareBank->loadChannelsFromAssetsDirectory("hectare/splits",16);

	mChannelBank = make_shared<LiveChannelBank>();

	//setHectare();
	setBeauford();

}


void PlayerApp::clearChannels() {
	if (mPlayers.size() > 0) {
		
		for (auto& p : mPlayers) {
			p->release();
		}
		mPlayers.clear();
		mChannelNames.clear();
		mScenes.clear();
		mVoiceChannels.clear();
		mProbabilities.clear();
	}
}

void PlayerApp::loadChannels() {
	
	int count = 0;
	for (auto& channel : mChannelBank->getChannels()) {
	
		SequencedSampleRef s = mAudioManager->getSequencedSample(true);
		mClock->clock(s->getSequencer());

		s->setChannel(channel, mScene);
		s->getSequencer()->start();
		mChannelNames.push_back(channel->getName());
		mVoiceChannels.push_back(count);
		mScenes.push_back(mScene);
		mPlayers.push_back(s);
		mProbabilities.push_back(1.0);
		s->setRmsMult(mRmsMult);
		count++;
	}

	mClock->setBPM(mBPM, mChannelBank->getTicks());
	mClock->start();
	
}

void PlayerApp::setBeauford() {
	clearChannels();
	mClock->stop();
	mChannelBank = mBeaufordBank;
	mBPM = 76;
	loadChannels();
}

void PlayerApp::setHectare() {
	clearChannels();

	mChannelBank = mHectareBank;
	mBPM = 140;
	loadChannels();
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

	if (event.getCode() == event.KEY_SPACE) {
		mShowMenu = !mShowMenu;
	}
}

bool PlayerApp::inspector()
{

	if (mShowMenu) {


		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		
		if (ui::Button("Beauford")) { setBeauford(); }
		ui::SameLine();
		if (ui::Button("Hectare")) { setHectare(); }


		if (ui::CollapsingHeader("Globals")) {

			if (ui::DragInt("Beats Per Minute", &mBPM, 1, 10, 250)) {
				mClock->setBPM(mBPM);
			}

			if (ui::DragFloat("RMS Mult", &mRmsMult, .1, 0, 40)) {
				for (auto& p : mPlayers) {
					p->setRmsMult(mRmsMult);
				}
			}

			if (ui::DragInt("Global Scene", &mScene, 1, 0, mChannelBank->getMaxScenes() - 1)) {
				int count = 0;
				for (auto& p : mPlayers) {
					mScenes.at(count) = mScene;
					p->setScene(mScene);
					count++;
				}
			}

			static float rate = 1;
			if (ui::DragFloat("Rate", &rate, 0.001, -1, 1)) {
				for (auto& p : mPlayers) {
					p->setRate(rate);
				}
			}

			static float interval = 0;
			if (ui::DragFloat("Interval", &interval, 1, -36, 36)) {
				for (auto& p : mPlayers) {
					p->setInterval(interval);
				}
			}
			
			static float probability = 0;
			if (ui::DragFloat("Reverse Probability", &probability, 0.001, 0, 1)) {
				for (auto& p : mPlayers) {
					p->setReverseProbability(probability);
				}
			}

			if (ui::DragFloat("Global Probability", &mAllProb, .01, 0, 1)) { setAllProbability(mAllProb); }
			if (ui::DragFloat("Global Spread", &mAllSpread, .01, 0, 1)) { setAllSpread(mAllSpread); }
			if (ui::DragFloat("Global Spread Probability", &mAllSpreadProb, .01, 0, 1)) { setAllSpreadProb(mAllSpreadProb); }
			if (ui::Button("Scene Up")) { sceneOffset(+1); }
			ui::SameLine();
			if (ui::Button("Scene Down")) { sceneOffset(-1); }

			if (ui::Button("Randomize Scenes")) { randomizeScenes(); }
			if (ui::Button("Randomize Probability")) { randomizeProbabilities(); }
			if (ui::Button("Randomize Slice Spread")) { randomizeSpread(); }

			if (ui::Button("Fade In")) {
				for (auto& p : mPlayers) p->fadeIn(1);
			}
			ui::SameLine();
			if (ui::Button("Fade Out")) {
				for (auto& p : mPlayers) p->fadeOut(1);
			}
		}

		
		int index = 0;
		for (const auto& player : mPlayers ) {
			ui::ScopedId pushId(index);
			std::string name = std::to_string(index) + ":" + player->getName();

			if (ui::CollapsingHeader(name.c_str())) {

				if (ui::Combo("Channel Selection",&mVoiceChannels[index], mChannelNames)) {
					std::string name = mChannelNames.at(mVoiceChannels[index]);
					LiveChannelRef ref = mChannelBank->getChannel(name);
					mPlayers.at(index)->setChannel(ref,mScenes[index]);
				}
				if (ui::DragInt("Scene", &(mScenes[index]), 1, 0, 9)) {
					player->setScene(mScenes[index]);
				}

				if (ui::Button("Fade In")) mPlayers.at(index)->fadeIn(.5);
				ui::SameLine();
				if (ui::Button("Fade Out")) mPlayers.at(index)->fadeOut(.5);
				ui::DragFloat("Interval", &(player->mInterval), 1, -24, 24);
				ui::DragFloat("Rate", &(player->mRate), 1, -1, 1);
				ui::DragInt("Clock Division", &(player->getSequencer()->mClockDivision), 1, 1, 32);
				ui::DragInt("Beat Offset", &(player->getSequencer()->mBeatOffset), 4, 0, 64);
				ui::DragFloat("Probability", &(player->getSequencer()->mProbability), 0.01, 0, 1);
				ui::DragFloat("Reverse Probability", &(player->mReverseProb), .001, 0, 1);
				ui::DragFloat("Slice Spread", &(player->mSliceSpread), .01, 0, 1);
				ui::DragFloat("Slice Spread Probability", &(player->mSpreadProb), .01, 0, 1);
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
				ui::DragFloat("Low Pass Res", &(player->mLowPassRes), 1, -64, 20);
				ui::DragFloat("High Pass Freq", &(player->mHighPassFreq), 10, 0, 20000);
				ui::DragFloat("High Pass Res", &(player->mHighPassRes), 1, -64, 20);
	
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

void PlayerApp::randomizeScenes() {
	for (auto& s : mScenes) {
		s = Rand::randInt(0, mChannelBank->getMaxScenes());
	}
	int count = 0;
	for (auto& p : mPlayers) {
		p->setScene(mScenes.at(count));
		count++;
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

void PlayerApp::setAllSpreadProb(float value) {
	for (auto& player : mPlayers) {
		player->setSliceSpreadProbability(value);
	}
}

void PlayerApp::sceneOffset(int count) {
	for (int i = 0; i < mScenes.size(); i++) {
		mScenes.at(i) = wrap( mScenes.at(i)+count, 0, mChannelBank->getMaxScenes());
		mPlayers.at(i)->setScene(mScenes.at(i));
	}
}

void PlayerApp::draw()
{

	//inspector();
	gl::clear( Color( 0, 0, 0 ) ); 
	
	size_t instruments = mPlayers.size();
	vec2 windowSize = app::getWindowSize();
	float width = windowSize.x / instruments;
	int count = 0;
	for (auto& p : mPlayers) {
		float rms =  p->getRms();
		float prob = p->getSequencer()->getProbability();
		float spread = p->getSliceSpread();
		float activeMult = .1+p->isActive()*.9;
		gl::color( prob *activeMult, rms, rms, 1);
		gl::drawSolidRect(Rectf(
			width*count+5,
			windowSize.y,
			width*(count+.5),
			windowSize.y-(windowSize.y*prob)));
		gl::color(rms, spread *activeMult, rms, 1);
		gl::drawSolidRect(Rectf(
			width*(count + .5),
			windowSize.y,
			width*(count + 1) - 5,
			windowSize.y - (windowSize.y*spread)));
		count++;
	}
	

}

int PlayerApp::wrap(int kX, int const kLowerBound, int const kUpperBound)
{
	int range_size = kUpperBound - kLowerBound + 1;

	if (kX < kLowerBound)
		kX += range_size * ((kLowerBound - kX) / range_size + 1);

	return kLowerBound + (kX - kLowerBound) % range_size;
}

CINDER_APP( PlayerApp, RendererGl )
