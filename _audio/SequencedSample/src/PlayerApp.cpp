#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "AudioManager.hpp"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "PolySamplePlayer.hpp"
#include "SequencedSample.h"
#include "NoteSequencer.h"
#include <iostream>


using namespace ci;
using namespace ci::app;
using namespace std;

class PlayerApp : public App {

  public:

	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void keyDown(KeyEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;
	void setProbability(MouseEvent event);
	void playNote(NoteEvent& event);
	
	vector<SequencedSampleRef> mSamplePlayers;

	AudioManagerRef mAudioManager;
	SampleBankRef mBuffers;


	std::vector<PatternRef> mPatterns;

	SequencerRef mClock;

	std::set<int> mAllNotes;
	std::map<int, int>								mNoteLookup;
	ci::audio::BufferRef							mBuffer;
	std::map<std::string, SampleBankRef>			mAllBanks;


	SampleBankRef mRootBank;
	BufferInfoRef mInfo;

};

void PlayerApp::setup()
{
	Rand::randomize();

	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../common/audio" );
	app::addAssetDirectory( commonAudioPath );

	mAudioManager = std::make_shared<AudioManager>();
	mAudioManager->setVolume(.3);
	mBuffers = std::make_shared<SampleBank>();
	mClock = std::make_shared<Sequencer>();

	mClock->start();
	mClock->setBPM(100, 64);

	mAllBanks = SampleBank::loadAllFilesInAssets();
	console() << "LOADED ALL ASSETS, FOUND " << mAllBanks.size() << std::endl;


	//mSamples = mAllBanks.at(0)->getBuffersWithInfo();

	
	std::vector<BufferInfoRef> infoSamples;
	for (auto& bank : mAllBanks) {
		for( auto& bufferInfo : bank.second->getBuffersWithInfo()){
			SequencedSampleRef s = mAudioManager->getSequencedSample(true);
			s->setBufferInfo(bufferInfo);
			mSamplePlayers.push_back(s);
			infoSamples.push_back(bufferInfo);
		}
	}

	for (auto& p : mSamplePlayers) {
		p->getSequencer()->setProbability(.25);
		mClock->clock( p->getSequencer() );
	}
		
}

void PlayerApp::playNote(NoteEvent& event) {

}

void PlayerApp::mouseDown( MouseEvent event )
{
	setProbability(event);
}

void PlayerApp::mouseDrag(MouseEvent event)
{
	setProbability(event);
}

void PlayerApp::setProbability(MouseEvent event) {
	float normPos = ci::clamp(float(event.getX()) / app::getWindowWidth(),0.0f,.99f);
	int player = std::floor(normPos * mSamplePlayers.size());
	mSamplePlayers.at(player)->getSequencer()->setProbability(1 - (float(event.getY()) / app::getWindowHeight()));
}

void PlayerApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	if (c == '1') { for (auto& p : mSamplePlayers) { p->setPattern(0); } }
	else if	(c == '2') {
		for (auto& p : mSamplePlayers) {
			p->setPattern(1);
		}
	}
	else if (c == '3') {
		for (auto& p : mSamplePlayers) {
			p->setPattern(2);
		}
	}
	else if (c == '4') {
		for (auto& p : mSamplePlayers) {
			p->setPattern(3);
		}
	}
	else if (c == '5') {
		for (auto& p : mSamplePlayers) {
			p->setPattern(4);
		}
	}
	

}

void PlayerApp::update()
{
	mClock->update();
	mAudioManager->update();

}

void PlayerApp::draw()
{
	gl::clear();
	int div = mSamplePlayers.size();
	vec2 winSize = app::getWindowSize();
	float xsize = float(winSize.x) / div;
	int count = 0;
	for (auto& p : mSamplePlayers) {
		float r = clamp(p->getRms() * 3,0.0f,1.0f);
		float prob = p->getSequencer()->getProbability();
		gl::color(prob, r,0, 1);
		gl::drawSolidRect(Rectf((xsize*count)+2, winSize.y, (xsize*(count + 1))-2, winSize.y-(winSize.y*prob)));
		gl::color(0, 0, 0, 1);

		//gl::drawSolidRect(Rectf(xsize*count, winSize.y, xsize*(count + 1), winSize.y-(winSize.y*r)));
		count++;
	}
	
}


CINDER_APP( PlayerApp, RendererGl )
