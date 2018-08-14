#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/GenNode.h"
#include "osc.h"
#include "cinder/audio/GainNode.h"
#include "AudioManager.hpp"
#include "SampleBank.h"
#include "GrainNode.h"
#include "SampleNode.h"
#include "CinderImGui.h"
#include "Recorder.hpp"
#include "cinder/Log.h"
#include "AudioDrawUtils.h"
#include "cinder/audio/ChannelRouterNode.h"
#include "EnvelopeFollowerNode.h"
#include "ComparatorNode.h"
#include "ClockNode.h"
#include "SequencerNode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using Receiver = osc::ReceiverUdp;
using protocol = asio::ip::udp;

class PlayerApp : public App {
  public:

	  PlayerApp();

	void setup() override;
	void setupAudioDevice();
	void setupOSC();
	void fileDrop(FileDropEvent event) override;
	void mouseUp(MouseEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void mouseMove(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void keyDown(KeyEvent event);
	void update() override;
	bool inspector();
	void draw() override;

	SampleNodeRef mPlayer;
	AudioManagerRef mAudioManager;
	SampleBankRef mRootBank;

	ci::audio::GenPhasorNodeRef mLfo;
	ci::audio::GainNodeRef mLfoMult;
	SequencerNodeRef mSeq;

	AudioRecorderRef mRecorder;


	ci::audio::InputDeviceNodeRef mInputDeviceNode;
	ci::audio::MonitorNodeRef	mInputMonitor;
	ci::audio::ChannelRouterNodeRef mChannelRouter;
	ci::audio::ChannelRouterNodeRef mRecRouter;

	ci::audio::ChannelRouterNodeRef mTriggerInputNode;
	EnvelopeFollowerNodeRef mEnvFolNode;
	ci::audio::MonitorNodeRef mAverageMonitor;
	ComparatorNodeRef mComparator;
	ClockNodeRef mMasterClock;
	SequencerNodeRef mPitchSeq;
	ComparatorNodeRef mTrigComparator;
	

	Receiver mReceiver;

	bool mShowMenu = true;
};

const uint16_t localPort = 4000;

PlayerApp::PlayerApp()
	: mReceiver(localPort)
{
}

void PlayerApp::setup()
{
	ui::initialize();
	auto ctx = ci::audio::master();

	Rand::randomize();

	setupAudioDevice();

	mRecRouter = ctx->makeNode( new ci::audio::ChannelRouterNode() );
	mInputMonitor = ctx->makeNode(new ci::audio::MonitorNode());
	mAverageMonitor = ctx->makeNode(new ci::audio::MonitorNode());
	mChannelRouter = ctx->makeNode(new ci::audio::ChannelRouterNode());
	mEnvFolNode = ctx->makeNode(new EnvelopeFollowerNode(ci::audio::Node::Format().channels(2).channelMode(ci::audio::Node::ChannelMode::MATCHES_INPUT)));
	mEnvFolNode->setMultiplier(5);
	mComparator = ctx->makeNode(new ComparatorNode(ci::audio::Node::Format().channels(2)));
	mMasterClock = ctx->makeNode(new ClockNode());
	mSeq = ctx->makeNode(new SequencerNode());
	mPitchSeq = ctx->makeNode(new SequencerNode());
	mTrigComparator = ctx->makeNode(new ComparatorNode(ci::audio::Node::Format().channels(1)));
	mTrigComparator->setThreshold(.15);

	int numInputChannels = mInputDeviceNode->getNumChannels();
	if (mInputDeviceNode && numInputChannels > 0) {
		ci::app::console() << "INPUT " << mInputDeviceNode->getName() << std::endl;
		mInputDeviceNode >> mInputMonitor;
		mInputDeviceNode->enable();

	}



	//ci::app::console() << app::getAppPath() << std::endl;
	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../../Media/audio");
	if( fs::exists( commonAudioPath ) ) app::addAssetDirectory( commonAudioPath );
	mRootBank = make_shared<SampleBank>();
	mRootBank->loadAssetDirectoryByName("test");
	ci::audio::BufferRef buffer = mRootBank->getRandomBuffer();

	mPlayer = ctx->makeNode( new SampleNode(ci::audio::Node::Format().channels(2)) );
	mPlayer >> ctx->getOutput();
	mPlayer->setBuffer( buffer );
	mRecorder = mAudioManager->getAudioRecorder();
	
	mLfo = ctx->makeNode( new ci::audio::GenPhasorNode() );
	mLfo->setFreq( -20 );
	//mPlayer->getRateParam()->svketProcessor(mLfo);
	mLfo->enable();

	std::list<int> scale = { 0,5 };
	mPlayer->setScale(scale);
	mPlayer->enable();
	mPlayer->setVolume(1);


	std::vector<float> sequence = { 1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,1,0 };
	mSeq->setSequence(sequence);
	std::vector<float> pitchseq = { .1f,0.0f,.36f,.8f,.5f,.7f,.9f,.84f,.6f };
	mPitchSeq->setSequence(pitchseq);

	mMasterClock >> mPitchSeq;
	mPitchSeq->setClockDivision(1);
	mMasterClock->enable();
	mMasterClock->setRate(0.15);
	mMasterClock >> mSeq;

	mTriggerInputNode = ctx->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(1)));
	if (numInputChannels > 2) {

		mInputDeviceNode >> mTrigComparator >> mTriggerInputNode->route(0,0);
		mPlayer->getTriggerParam()->setProcessor(mTriggerInputNode);
		mTriggerInputNode >> mAverageMonitor;

	}
	else {

		mPlayer->getTriggerParam()->setProcessor(mSeq);
		mSeq >> mAverageMonitor;
	}

	//mPlayer->getTriggerParam()->setProcessor(mSeq);
	//mTriggerInputNode >> mAverageMonitor;

	//mTriggerInputNode->enable();
	mTriggerInputNode->enable();


	mPlayer->getPositionParam()->setProcessor(mPitchSeq);
	mAverageMonitor->enable();
	mRecorder->attachTo( mPlayer );
	mRecorder->record( false );

	ctx->enable();

	setupOSC();
	
}

void PlayerApp::setupAudioDevice()
{
	// debug print all devices to console
	console() << audio::Device::printDevicesToString() << endl;

	auto ctx = audio::master();
	audio::DeviceRef deviceIn;
	audio::DeviceRef deviceOut;

	audio::DeviceRef es8 = audio::Device::findDeviceByName("Microphone (ES-8)");
	if (es8) {
		deviceIn = es8;
		deviceOut = audio::Device::findDeviceByName("Speakers (ES-8)");
	}

	audio::DeviceRef kmix = audio::Device::findDeviceByName("Line (3- K-MixUSBAudioDriver)");
	if (kmix) {
		deviceIn = kmix;
		deviceOut = audio::Device::findDeviceByName("Speakers (3- K-MixUSBAudioDriver)");
	}

	audio::DeviceRef mill_device = audio::Device::findDeviceByName("Speakers (High Definition Audio Device)");
	if (mill_device) {
		deviceIn = nullptr;
		deviceOut = mill_device;
	}

	if (!mill_device && !kmix && !es8) {
		for (const auto &dev : audio::Device::getDevices()) {
			if (!deviceOut || deviceOut->getNumOutputChannels() < dev->getNumOutputChannels()) deviceOut = dev;
			if (!deviceIn || deviceIn->getNumInputChannels() < dev->getNumInputChannels()) deviceIn = dev;
		}
	}

	if (deviceIn && deviceIn->getNumInputChannels()>0)	mInputDeviceNode = ctx->createInputDeviceNode(deviceIn, audio::Node::Format().channels(deviceIn->getNumInputChannels()));
	auto out = ctx->createOutputDeviceNode(deviceOut, audio::Node::Format().channels(deviceOut->getNumOutputChannels()));
	ctx->setOutput(out);
	
	getWindow()->setTitle(deviceOut->getName());

}

void PlayerApp::setupOSC() {

	mReceiver.setListener("/4/xy",
		[&](const osc::Message &msg) {
		mPlayer->setTriggerSpeed(.01+msg[0].flt());
		mPlayer->setPosition(msg[1].flt());
	});

	mReceiver.setListener("/4/toggle1",
		[&](const osc::Message &msg) {
		mPlayer->gate( msg[0].flt() );
	});


	try { mReceiver.bind();	}
	catch (const osc::Exception &ex) {
		CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
		quit();
	}

	mReceiver.listen(
		[](asio::error_code error, protocol::endpoint endpoint) -> bool {
		if (error) {
			CI_LOG_E("Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint);
			return false;
		}
		else
			return true;
	});

	
}

void PlayerApp::fileDrop(FileDropEvent event)
{
	for (auto& f : event.getFiles()) {
		mRootBank->addBuffer(f);
	}
}

void PlayerApp::mouseUp(MouseEvent event)
{
	//mPlayer->gate(false);
}

void PlayerApp::mouseDown( MouseEvent event )
{
	//mPlayer->gate(true);
}

void PlayerApp::mouseMove(MouseEvent event) {

	float relX = ci::clamp( event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1-(event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	//mPlayer->setPosition(relX);
	//mPlayer->setInterval(std::floor(72 * (relY - .5)));
}

void PlayerApp::mouseDrag(MouseEvent event) {

	float relX = ci::clamp(event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1 - (event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);
	//mPlayer->setPosition(relX);
	//mPlayer->setInterval(std::floor(72 * (relY - .5)));

}

void PlayerApp::keyDown(KeyEvent event) {
	char c = event.getChar();
	switch (c) {

	case 'r':
		mRecorder->toggleRecord();
		if (!mRecorder->isRecording()) {
			mPlayer->gate(false);
			mPlayer->setBuffer(mRecorder->getRecordedCopy());

			ci::app::console() << "Finished recording " << std::endl;
		}
		else {
			ci::app::console() << "Started recording " << std::endl;
		}
		break;

	case 'n':
		mPlayer->setBuffer(mRootBank->getNextBuffer());
		break;

	}
}

void PlayerApp::update()
{
	//mClock->update();
	inspector();
	//mPlayer->setLength(mClockNode->getRate());
	//console() << mPlayer->getNumActiveGrains() << std::endl;
}

bool PlayerApp::inspector()
{
	if (mShowMenu) {

		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		ui::SetWindowFontScale(getDisplay()->getWidth()/1920.0f);

		float clockrate = mMasterClock->getRate();
		if (ui::DragFloat("Clock Rate", &clockrate, .001, 0, 1)) {
			mMasterClock->setRate(clockrate);
		}

		float clockjitter = mMasterClock->getRateJitter();
		if (ui::DragFloat("Clock Jitter", &clockjitter, .001, 0, 4)) {
			mMasterClock->setRateJitter(clockjitter);
		}

		int clockdivs = mMasterClock->getClockDivisions();
		if (ui::DragInt("Clock Divisions", &clockdivs, 1, 1, 8)) {
			mMasterClock->setClockDivisions(clockdivs);
		}
		ui::DragFloat("Trigger Rate", &(mPlayer->mTriggerRate), .001, 0, 1);
		ui::DragFloat("Trigger Rate Jitter", &(mPlayer->mTriggerRateJitter), .001, 0, 1);
		float pos = mPlayer->getPosition();
		if (ui::DragFloat("Position", &pos, .001, 0, 1)) { mPlayer->setPosition(pos); }
		ui::DragFloat("Position Jitter", &(mPlayer->mPositionJitter), .001, 0, 10);
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
		ui::DragFloat("Ramp", &(mPlayer->mRamp), .001, .001,  1);

		bool record = mRecorder->isRecording();
		if (ui::Checkbox("Record", &record)) {
			mRecorder->record(record);
		}

		int samples = mSeq->getDelaySize();
		if (ui::DragInt("Delay Size", &samples, 100, 10 * ci::audio::master()->getSampleRate())) {
			mSeq->setDelaySize(samples);
		}

		float speed = mLfo->getFreq();
		if (ui::DragFloat("LFO Freq", &speed, .001, -10, 10)) {
			mLfo->setFreq(speed);
		}
	}
	return 1;
}

void PlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	if (mEnvFolNode &&
		mEnvFolNode->getNumChannels() > 0 &&
		mAverageMonitor &&
		mAverageMonitor->getNumConnectedInputs()) {

		Rectf scopeRect(getWindowWidth() - 610, 10, getWindowWidth() - 10, 100);
		drawAudioBuffer(mAverageMonitor->getBuffer(), scopeRect, true);
		//ci::app::console() << mAverageMonitor->getBuffer()[0] << std::endl;

	}
	
	/*

	// Draw the Scope's recorded Buffer in the upper right.
	if (mInputDeviceNode && 
		mInputDeviceNode->getNumChannels() > 0 && 
		mInputMonitor && 
		mInputMonitor->getNumConnectedInputs()) {

		Rectf scopeRect(getWindowWidth() - 610, 10, getWindowWidth() - 10, 510);
		drawAudioBuffer(mInputMonitor->getBuffer(), scopeRect, true);

	}
	*/
		


}

CINDER_APP( PlayerApp, RendererGl )
