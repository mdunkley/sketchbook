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

struct SeqChannel {
	PolySamplePlayerRef player;
	SequencerRef  seq;
	bool active;
};


// Number keys 1 - 5 to turn voices on and off

class SliceSequenceApp : public App {

  public:

	void prepareSettings(Settings *settings);
	void setup() override;
	void keyDown(KeyEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	int quantizeMidiNote(float n, vector<int> s);
	void initializeSequences();

	AudioManagerRef mAudioManager;

	std::vector<SeqChannel> mChannels;

	SampleBankRef mSampleBank;
	SampleBankRef mKeySequences;

	SequencerRef mClock;

	void triggerVoice(StepInfo s, int channel=0);
	void triggerNote(StepInfo s, int channel = 0);
	void triggerChord(StepInfo s, int channel=0);


	std::vector<int> mScale = { 0,3,5,7,10 };
	std::vector<vector<int>> mTriggerSequences;
	std::vector<vector<double>> mModulationSequences;

};

void SliceSequenceApp::prepareSettings(Settings *settings)
{
	settings->setHighDensityDisplayEnabled(true);
}

void SliceSequenceApp::setup()
{

	initializeSequences();

	// Make manager, banks, and sequencers
	mAudioManager = std::make_shared<AudioManager>();
	mSampleBank = std::make_shared<SampleBank>();
	mKeySequences = std::make_shared <SampleBank>();
	mClock = std::make_shared<Sequencer>();
	
	for (int i = 0; i < 8; i++) {
		SeqChannel chan{
			mAudioManager->getPolySamplePlayer(true),
			make_shared<Sequencer>(),
			true
		};
		mChannels.emplace_back(chan);
	}

	for (auto& c : mChannels) {
		mClock->clock(c.seq);
	}

	// Load samples
	mSampleBank->loadAssetDirectory("bank_00");
	mKeySequences->loadAssetDirectory("key_sequences");

	// Setup core clock
	mClock->setBPM(120, 64);
	mClock->start();
	
	// Randomize the randomizer
	Rand::randomize();

	// Setup channel 0
	int numSlices = 16;
	mChannels.at(0).seq->getTriggerSignal().connect( [&](StepInfo s) { SliceSequenceApp::triggerVoice(s,0); } );
	mChannels.at(0).seq->setClockDivision(16);
	mChannels.at(0).seq->setTriggerSequence( mTriggerSequences.at( Rand::randInt(mTriggerSequences.size() ) ) );
	mChannels.at(0).seq->addModulationSequence( mModulationSequences.at(Rand::randInt(mModulationSequences.size())) );
	mChannels.at(0).player->setBuffer(mSampleBank->getBuffer(3));
	mChannels.at(0).player->setSliceDivisions(16);
	mChannels.at(0).player->setHold(mChannels.at(0).seq->getStepLength()/.125);
	mChannels.at(0).player->setRelease(.02);


	numSlices = 32;
	mChannels.at(1).seq->getTriggerSignal().connect([&](StepInfo s) { SliceSequenceApp::triggerVoice(s,1); });
	mChannels.at(1).seq->setClockDivision(128);
	mChannels.at(1).seq->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
	mChannels.at(1).seq->triggerModulation(true);
	mChannels.at(1).player->setBuffer(mSampleBank->getBuffer(2));
	mChannels.at(1).player->setSliceDivisions(numSlices);
	mChannels.at(1).player->setRelease(.02);

	numSlices = 32;
	mChannels.at(2).seq->getTriggerSignal().connect([&](StepInfo s) { SliceSequenceApp::triggerVoice(s,2); });
	mChannels.at(2).seq->setClockDivision(64);
	mChannels.at(2).seq->addModulationSequence(mModulationSequences.at(Rand::randInt(mModulationSequences.size())));
	mChannels.at(2).seq->setTriggerSequence( mTriggerSequences.at(Rand::randInt(mTriggerSequences.size())) );
	mChannels.at(2).seq->triggerModulation(true);
	mChannels.at(2).player->setBuffer(mSampleBank->getBuffer(1));
	mChannels.at(2).player->setSliceDivisions(numSlices);
	mChannels.at(2).player->setRelease(.02);

	numSlices = 128;
	mChannels.at(3).seq->getTriggerSignal().connect([&](StepInfo s) { SliceSequenceApp::triggerNote(s,3); });
	mChannels.at(3).seq->setClockDivision(8);
	mChannels.at(3).seq->addModulationSequence( mModulationSequences.at(Rand::randInt(mModulationSequences.size())) );
	mChannels.at(3).seq->setTriggerSequence(mTriggerSequences.at(Rand::randInt(mTriggerSequences.size())));
	mChannels.at(3).seq->triggerModulation(true);
	mChannels.at(3).seq->setProbability(.35);
	mChannels.at(3).player->setBuffer(mKeySequences->getBuffer(4));
	mChannels.at(3).player->setSliceDivisions(numSlices);
	mChannels.at(3).player->setRelease(.02);

	
	// Setup channel 4
	// keyboard mode, one slice per note of the midi scale

	numSlices = 128;
	mChannels.at(4).seq->getTriggerSignal().connect([&](StepInfo s) { SliceSequenceApp::triggerChord(s,4); });

	// Note sequences
	std::vector<double> stepSeq4a;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4a.push_back(48 + Rand::randInt(36));
	std::vector<double> stepSeq4b;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4b.push_back(48 + Rand::randInt(36));
	std::vector<double> stepSeq4c;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4c.push_back(48 + Rand::randInt(36));

	// Volume sequences
	std::vector<double> stepSeq4d;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4a.push_back(Rand::randFloat());
	std::vector<double> stepSeq4e;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4b.push_back(Rand::randFloat());
	std::vector<double> stepSeq4f;
	for (int i = 0; i < Rand::randInt(3,20); i++) stepSeq4c.push_back(Rand::randFloat());

	mChannels.at(4).seq->setClockDivision(8);
	mChannels.at(4).seq->setTriggerSequence(mTriggerSequences.at(Rand::randInt(mTriggerSequences.size())));
	mChannels.at(4).seq->addModulationSequence(stepSeq4a);
	mChannels.at(4).seq->addModulationSequence(stepSeq4b);
	mChannels.at(4).seq->addModulationSequence(stepSeq4c);
	mChannels.at(4).seq->addModulationSequence(stepSeq4d);
	mChannels.at(4).seq->addModulationSequence(stepSeq4e);
	mChannels.at(4).seq->addModulationSequence(stepSeq4f);
	mChannels.at(4).player->setBuffer( mKeySequences->getBuffer(1) );
	mChannels.at(4).player->setSliceDivisions(numSlices);
	mChannels.at(4).player->setRelease(.02);
	
}

void SliceSequenceApp::initializeSequences() {

	std::vector<int> trigSeq0 = { 1,0,0,1,0,0,1,0 };
	std::vector<int> trigSeq1 = { 1,0,0,0 };
	std::vector<int> trigSeq2 = { 1,0 };
	std::vector<int> trigSeq3 = { 1,0,0,0,0,0,1,0 };
	std::vector<int> trigSeq4 = { 0,0,1,0 };
	std::vector<int> trigSeq5 = { 1,1,1,1,1,1,1,0 };
	std::vector<int> trigSeq6 = { 1,1,0,0 };
	std::vector<int> trigSeq7 = { 0,0,0,0,0,0,0,1 };
	mTriggerSequences = { trigSeq0, trigSeq1, trigSeq2, trigSeq3, trigSeq4, trigSeq5, trigSeq6, trigSeq7 };

	std::vector<double> modSeq0 = { 0,.25,.5,.75 };
	std::vector<double> modSeq1 = { 1,.25,.35,.25,1,.35,.55,.25 };
	std::vector<double> modSeq2 = { .5,1,0,0 };
	std::vector<double> modSeq3 = { 0,.9,.4,.3,0,.2,.4,.9 };
	std::vector<double> modSeq4 = { 1,.9,.4,0,1,.2,.4,.9 };
	std::vector<double> modSeq5 = { .1,.3,.8 };
	mModulationSequences = { modSeq0,modSeq1,modSeq2,modSeq3, modSeq4, modSeq5 };


}

void SliceSequenceApp::triggerVoice(StepInfo s, int channel) {

	//console() << "Voice "<<channel<< " is being triggered " << std::endl;
	if ( mChannels.at(channel).active ) {
		int sliceCount = mChannels.at(channel).player->getNumSlices();
		mChannels.at(channel).player->setSlice( int(s.values.at(0)*sliceCount) );
		mChannels.at(channel).player->setHold(.35);
		mChannels.at(channel).player->trigger();
	}
}

void SliceSequenceApp::triggerNote(StepInfo s, int channel) {
	if (mChannels.at(channel).active) {
		int note = quantizeMidiNote(floor(48 + s.values.at(0) * 24), mScale);
		mChannels.at(channel).player->setSlice(note);
		mChannels.at(channel).player->setHold(.1f);
		mChannels.at(channel).player->setRelease(Rand::randFloat(.5));
		mChannels.at(channel).player->trigger();
	}
}

void SliceSequenceApp::triggerChord(StepInfo s, int channel) {
	if (mChannels.at(channel).active) {

		mChannels.at(channel).player->setBuffer(mKeySequences->getBuffer(1));
		mChannels.at(channel).player->setSlice( quantizeMidiNote( s.values.at(0), mScale ) );
		mChannels.at(channel).player->setVolume( s.values.at(3) );
		mChannels.at(channel).player->setHold(.1f);
		mChannels.at(channel).player->setRelease(Rand::randFloat(.5));
		mChannels.at(channel).player->trigger();

		mChannels.at(channel).player->setSlice(quantizeMidiNote(s.values.at(1), mScale));
		mChannels.at(channel).player->setVolume(s.values.at(4));
		mChannels.at(channel).player->setRelease(Rand::randFloat(.5));
		if(Rand::randFloat()>.8) mChannels.at(channel).player->setBuffer(mKeySequences->getBuffer(5));
		mChannels.at(channel).player->trigger();

		mChannels.at(channel).player->setBuffer(mKeySequences->getBuffer(5));
		mChannels.at(channel).player->setSlice(quantizeMidiNote(s.values.at(2), mScale));
		mChannels.at(channel).player->setVolume(s.values.at(5));
		mChannels.at(channel).player->setRelease(Rand::randFloat(.5));
		mChannels.at(channel).player->trigger();
	}
}


void SliceSequenceApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	if (c == '1')		mChannels.at(0).active = !mChannels.at(0).active;
	else if (c == '2')	mChannels.at(1).active = !mChannels.at(1).active;
	else if (c == '3')	mChannels.at(2).active = !mChannels.at(2).active;
	else if (c == '4')	mChannels.at(3).active = !mChannels.at(3).active;
	else if (c == '5')	mChannels.at(4).active = !mChannels.at(4).active;
}
void SliceSequenceApp::mouseDown( MouseEvent event )
{
}

void SliceSequenceApp::update()
{
	mClock->update();
	mAudioManager->update();

	for (auto& c : mChannels) {
		;
	}
}

void SliceSequenceApp::draw()
{
	gl::clear(Color(0, 0, 0));

	int panels = 5;
	vec2 s = app::getWindowSize();
	float w = s.x*1/panels;
	for (int i = 0; i < panels; i++) {
		float r = 0.0f;
		if (mChannels.at(i).player) r = mChannels.at(i).player->getRms() * 5.0f;
		gl::color(r, r, r, 1);
		gl::drawSolidRect(Rectf(w*i, 0, w*(i+1), s.y));
	}

}


int SliceSequenceApp::quantizeMidiNote(float n, vector<int> s) {

	vector<int> fullScale;

	fullScale.push_back(s.back() - 12);
	fullScale.insert(fullScale.end(), s.begin(), s.end());
	fullScale.push_back(s[0] + 12);

	int qn = floor(n);
	int interval = qn % 12;
	int octave = qn / 12;

	int minDist = 9999;
	int minVal = 9999;
	for (int i = 0; i<fullScale.size(); i++) {
		int dist = int(abs(interval - fullScale[i]));
		if (dist<minDist) {
			minVal = fullScale[i];
			minDist = dist;
		}
	}

	interval = minVal;
	return interval + (octave * 12);
}
CINDER_APP( SliceSequenceApp, RendererGl )
