#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"

#include "AudioManager.hpp"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "GranularPlayer.hpp"
#include "PolySamplePlayer.hpp"
#include "cinder/audio/FilterNode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MultiGrainApp : public App {

  public:

	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseUp(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;
	void randomize();

	AudioManagerRef mAudioManager;

	SequencerRef mClock;
	SequencerRef mChannel0;
	SequencerRef mChannel1;
	SequencerRef mChannel2;
	SequencerRef mChannel3;

	SequencerRef mTest;

	void print(int count);
	void playChannel0(StepInfo s);
	void playChannel1(StepInfo s);
	void playChannel2(StepInfo s);
	void playChannel3(StepInfo s);

	void randomizeBanks();

	SampleBankRef   mSampleBank0;
	SampleBankRef	mSampleBank1;
	SampleBankRef	mSampleBank2;
	SampleBankRef	mSampleBank3;
	SampleBankRef   mSampleBank4;
	SampleBankRef   mSampleBank5;
	SampleBankRef	mSampleBank6;

	SampleBankRef	mChannel0Bank;
	SampleBankRef	mChannel1Bank;
	SampleBankRef	mChannel2Bank;
	SampleBankRef	mChannel3Bank;

	bool doC0 = true;
	bool doC1 = true;
	bool doC2 = true;
	bool doC3 = true;

	std::vector<PolySamplePlayerRef> mPolySamplePlayers;
	std::vector<SampleVoiceRef> mSamplePlayers;

	PolySamplePlayerRef c0;
	PolySamplePlayerRef c1;
	PolySamplePlayerRef c2;
	PolySamplePlayerRef c3;

	SampleVoiceRef s0;
	SampleVoiceRef s1;
	SampleVoiceRef s2;
	SampleVoiceRef s3;

	std::vector<int> mDivisions = { 1,2,4,6,8,12,16,24,32,48,64 };
	std::vector<vector<int>> mTriggerSequences;
	std::vector<vector<double>> mModulationSequences;

	ci::audio::FilterLowPassNodeRef		mLowPassFilter;
	ci::audio::FilterHighPassNodeRef	mHighPassFilter;
};

void MultiGrainApp::setup()
{
	Rand::randomize();

	mAudioManager = std::make_shared<AudioManager>();

	// pull a list of players
	mAudioManager->getPolySamplePlayers(8);
	mAudioManager->getSampleVoices(64);
	
	// initialize SampleBanks
	mSampleBank0 = std::make_shared<SampleBank>();
	mSampleBank1 = std::make_shared<SampleBank>();
	mSampleBank2 = std::make_shared<SampleBank>();
	mSampleBank3 = std::make_shared<SampleBank>();
	mSampleBank4 = std::make_shared<SampleBank>();
	mSampleBank5 = std::make_shared<SampleBank>();
	mSampleBank6 = std::make_shared<SampleBank>();

	// target channel buffer bank references
	// one level of indirection for easier buffer combo experiments
	// currently set to randomize banks per channel on load
	int mode = 1;
	if (mode == 0) {
		mChannel0Bank = mSampleBank2;
		mChannel1Bank = mSampleBank0;
		mChannel2Bank = mSampleBank4;
		mChannel3Bank = mSampleBank5;
	}
	else if (mode == 1) {
		mChannel0Bank = mSampleBank0;
		mChannel1Bank = mSampleBank1;
		mChannel2Bank = mSampleBank2;
		mChannel3Bank = mSampleBank3;
	}

	// set up trigger sequences
	std::vector<int> t0 = { 1,0,0,0 };
	std::vector<int> t1 = { 1,0,1,0 };
	std::vector<int> t2 = { 0,0,1,0 };
	std::vector<int> t3 = { 1,0,0 };
	std::vector<int> t4 = { 1,0,0,0,0,1,0,0 };
	std::vector<int> t5 = { 1,0,0,1,0,1,0,0 };
	std::vector<int> t6 = { 1,1,1,0 };
	std::vector<int> t7 = { 0,0,0,1 };
	std::vector<int> t8 = { 1,0,1,0,1,0,0,0};
	mTriggerSequences = { t0,t1,t2,t3,t4,t5,t6,t7,t8 };

	
	std::vector<double> m0 = { 0,.5,.75,.95 };
	std::vector<double> m1 = { 1,.25,0,.25,1,.25,0,.33 };
	std::vector<double> m2 = { .25,.25,.5,.7,.9,0,.1,.2 };
	std::vector<double> m3 = { .5,.5,.5,.3,.3,.3,0,0,0,.1,.2 };

	mModulationSequences = { m0,m1,m2,m3 };

	for (int i = 0; i < 12; i++) {
		int count = Rand::randInt(16);
		std::vector<double> d;
		for (int j = 0; j < count; j++) {
			d.push_back(Rand::randFloat());
		}
		mModulationSequences.push_back(d);
	}

	
	// Build bank paths from local asset directories
	mSampleBank0->loadPath( getAssetDirectories()[0] / "bank_00");
	mSampleBank1->loadPath( getAssetDirectories()[0] / "bank_01");
	mSampleBank2->loadPath( getAssetDirectories()[0] / "bank_02");
	mSampleBank3->loadPath( getAssetDirectories()[0] / "bank_03");
	mSampleBank4->loadPath( getAssetDirectories()[0] / "bank_04");
	mSampleBank5->loadPath( getAssetDirectories()[0] / "bank_05");
	mSampleBank6->loadPath( getAssetDirectories()[0] / "bank_06");

	// Load up a master clock and four clocked sequencers
	mClock =	std::make_shared<Sequencer>();
	mChannel0 = std::make_shared<Sequencer>();
	mChannel1 = std::make_shared<Sequencer>();
	mChannel2 = std::make_shared<Sequencer>();
	mChannel3 = std::make_shared<Sequencer>();
	mTest =		std::make_shared<Sequencer>();

	// Channel Seqs all derive timing from mClock
	mClock->clock( mChannel0 );
	mClock->clock( mChannel1 );
	mClock->clock( mChannel2 );
	mClock->clock( mChannel3 );

	mClock->setBPM(120,16);
	mClock->start();

	// hook up sequencers to their targets
	mChannel0->getTriggerSignal().connect( [&](StepInfo m){ MultiGrainApp::playChannel0(m); });
	mChannel1->getTriggerSignal().connect( [&](StepInfo m){ MultiGrainApp::playChannel1(m); });
	mChannel2->getTriggerSignal().connect( [&](StepInfo m){ MultiGrainApp::playChannel2(m); });
	mChannel3->getTriggerSignal().connect( [&](StepInfo m){ MultiGrainApp::playChannel3(m); });

	// grab a preloaded sample instrument
	c0 = mAudioManager->getPolySamplePlayer();  
	// take it out of circulation
	c0->setInUse(true); 
	// set parameters on the voice
	c0->setHold(1.0);
	c0->setPosition(.1);
	c0->setPositionJitter(.01);
	c0->setPanJitter(.55);
	// possible to load files directly, but for now we've already got a bankfull
	//c0->setBuffer(mSampleBank0->add(audio::load(loadAsset("bank_01/1-KEY(CS.WAV"))));
	// Load a random buffer from SampleBank0
	c0->setBuffer(mChannel0Bank->getBuffers()[Rand::randInt(mChannel0Bank->getSize())]);


	randomize();

}

void MultiGrainApp::print(int count) {
	ci::app::console() << count << std::endl;
}

void MultiGrainApp::playChannel0(StepInfo s) {
	s0 = mAudioManager->getSampleVoice();
	s0->setInUse(true);
	s0->setAttack(.01f);
	s0->setHold(s.values.at(2));
	s0->setDecay(0.01f);
	s0->setVolume(s.values.at(1));
	s0->setBuffer(mChannel0Bank->getBufferByLookup(s.values.at(0)));
	s0->trigger();
}
void MultiGrainApp::playChannel1(StepInfo s) {
	//c1->setBuffer(mChannel1Bank->getRandomBuffer());
	//if(doC1) c1->trigger();
	s1 = mAudioManager->getSampleVoice();
	//ci::app::console() << "Playing Channel 2 with " << s2->getName() << std::endl;
	s1->setInUse(true);
	s1->setAttack(.01f);
	s1->setHold(s.values.at(2));
	s1->setDecay(0.01f);
	s1->setVolume(s.values.at(1));
	s1->setBuffer(mChannel1Bank->getBufferByLookup(s.values.at(0)));
	s1->trigger();
} 

void MultiGrainApp::playChannel2(StepInfo s) {
	//c2->setBuffer(mChannel2Bank->getRandomBuffer());
	//if(doC2) c2->trigger();
	
	s2 = mAudioManager->getSampleVoice();
	//ci::app::console() << "Playing Channel 2 with " << s2->getName() << std::endl;
	s2->setInUse(true);
	s2->setAttack(.01f);
	s2->setHold(s.values.at(2));
	s2->setDecay(0.01f);
	s2->setVolume(s.values.at(1));
	s2->setBuffer(mChannel2Bank->getBufferByLookup(s.values.at(0)));
	s2->trigger();
	
}

void MultiGrainApp::playChannel3(StepInfo s) {

	ci::app::console() << "Received C3 trigger " << s.clockCount << " " << s.trigger;
	for (auto c : s.values) {
		ci::app::console() << " " << c;
	}
	ci::app::console() << " and that is all" << std::endl;
	
	s3 = mAudioManager->getSampleVoice();
	ci::app::console() << "Playing Channel 3 with " << s3->getName() << std::endl;
	s3->setInUse(true);
	s3->setAttack(.01f);
	s3->setHold(s.values.at(2));
	s3->setDecay(0.01f);
	s3->setVolume(s.values.at(1));
	s3->setBuffer(mChannel2Bank->getBufferByLookup(s.values.at(0)));
	s3->trigger();
}

void MultiGrainApp::mouseDown( MouseEvent event )
{
	randomize();
}

void MultiGrainApp::randomize() {

	randomizeBanks();

	// Random buffers
	c0->setBuffer(mChannel0Bank->getRandomBuffer());

	// Randomize the sequencers
	for (auto c : { mChannel0,mChannel1,mChannel2,mChannel3 }) {

		// Random clock division
		c->setClockDivision(mDivisions[Rand::randInt(mDivisions.size())]);

		// Random trigger sequences
		c->setTriggerSequence(mTriggerSequences.at(Rand::randInt(mTriggerSequences.size())));

		// Add 4 modulation lanes from the bank
		c->clearModulationSequences();
		c->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
		c->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
		c->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
		c->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
	}

}

void MultiGrainApp::randomizeBanks() {

	vector<SampleBankRef> allbanks = { mSampleBank0,mSampleBank1,mSampleBank2,mSampleBank3,mSampleBank4,mSampleBank5,mSampleBank6 };
	//vector<SampleBankRef> allbanks = { mSampleBank6 };
	mChannel0Bank = allbanks.at(Rand::randInt(allbanks.size()));
	mChannel1Bank = allbanks.at(Rand::randInt(allbanks.size()));
	mChannel2Bank = allbanks.at(Rand::randInt(allbanks.size()));
	mChannel3Bank = allbanks.at(Rand::randInt(allbanks.size()));
}

void MultiGrainApp::mouseUp(MouseEvent event) {

}

void MultiGrainApp::mouseDrag(MouseEvent event) {

}

void MultiGrainApp::fileDrop(FileDropEvent event) {

	mSampleBank0->appendBuffers(event.getFiles());
	//ci::app::console() << event.getPos() << std::endl;
}

void MultiGrainApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	if (c == '1') doC0 = !doC0;
	if (c == '2') doC1 = !doC1;
	if (c == '3') doC2 = !doC2;
	if (c == '4') doC3 = !doC3;
	if (c == 'g') ci::app::console() << audio::master()->printGraphToString();
}

void MultiGrainApp::update(){

	// Only source clock requires update, derived clocks follow through signals
	mClock->update();
	mAudioManager->update();
}

void MultiGrainApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	vec2 s = app::getWindowSize();
	float w = s.x*.25;

	float r = 0.0f;
	if(s0) r = s0->getRms() * 5.0f;
	gl::color(r, r, r, 1);
	gl::drawSolidRect(Rectf(0, 0, w, s.y));

	r = 0.0;
	if(s1) r = s1->getRms()*5.0f;
	gl::color(r, r, r, 1);
	gl::drawSolidRect(Rectf(w, 0,w*2, s.y));

	r = 0.0;
	if(s2) r = s2->getRms() * 5.0f;
	gl::color(r, r, r, 1);
	gl::drawSolidRect(Rectf(w*2, 0, w * 3, s.y));

	r = 0.0;
	if(s3) r = s3->getRms() * 5.0f;
	gl::color(r, r, r, 1);
	gl::drawSolidRect(Rectf(w*3, 0, w * 4, s.y));
}

CINDER_APP( MultiGrainApp, RendererGl )
