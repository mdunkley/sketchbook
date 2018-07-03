#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/GenNode.h"
#include "osc.h"
#include "cinder/audio/GainNode.h"
#include "AudioManager.hpp"
#include "SampleBank.h"
#include "Sequencer.hpp"
#include "GrainNode.h"
#include "SampleNode.h"
#include "CinderImGui.h"
#include "Recorder.hpp"
#include "cinder/Log.h"
#include "AudioDrawUtils.h"
#include "cinder/audio/ChannelRouterNode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using Receiver = osc::ReceiverUdp;
using protocol = asio::ip::udp;

class PlayerApp : public App {
  public:

	  PlayerApp();

	void setup() override;
	void setupMultichannelDevice();
	void setupOSC();
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

	AudioRecorderRef mRecorder;
	ci::audio::InputDeviceNodeRef mInputDeviceNode;
	ci::audio::MonitorNodeRef	mInputMonitor;
	ci::audio::ChannelRouterNodeRef mChannelRouter;
	ci::audio::ChannelRouterNodeRef mRecRouter;

	SequencerRef mClock;

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

	setupMultichannelDevice();

	/*
	audio::DeviceRef indev = audio::Device::findDeviceByName("Microphone (ES-8)");
	if( indev ) mInputDeviceNode = ctx->createInputDeviceNode(indev, audio::Node::Format().channels(indev->getNumInputChannels()));
	audio::DeviceRef outdev = audio::Device::findDeviceByName("Speakers (ES-8)");
	if(outdev){
		audio::OutputDeviceNodeRef multichannelOutputDeviceNode = ctx->createOutputDeviceNode(outdev, audio::Node::Format().channels(outdev->getNumOutputChannels()));
		ctx->setOutput(multichannelOutputDeviceNode);
	}

	indev = audio::Device::findDeviceByName("Line (3- K-MixUSBAudioDriver)");
	if( indev ) mInputDeviceNode = ctx->createInputDeviceNode(indev, audio::Node::Format().channels(indev->getNumInputChannels()));
	outdev = audio::Device::findDeviceByName("Speakers (3- K-MixUSBAudioDriver)");
	if (outdev) {
		audio::OutputDeviceNodeRef multichannelOutputDeviceNode = ctx->createOutputDeviceNode(outdev, audio::Node::Format().channels(outdev->getNumOutputChannels()));
		ctx->setOutput(multichannelOutputDeviceNode);
	}
	*/

	mRecRouter = ctx->makeNode( new ci::audio::ChannelRouterNode() );
	mInputMonitor = ctx->makeNode(new ci::audio::MonitorNode());
	mChannelRouter = ctx->makeNode(new ci::audio::ChannelRouterNode());
	ci::app::console() << "INPUT " << mInputDeviceNode->getName() << std::endl;
	mInputDeviceNode >> mInputMonitor;
	mInputDeviceNode->enable();

	setupOSC();

	ci::app::console() << app::getAppPath() << std::endl;
	fs::path commonAudioPath = fs::canonical(app::getAppPath() / "../../../../../../Media/audio");
	app::addAssetDirectory( commonAudioPath );
	mRootBank = make_shared<SampleBank>();
	mRootBank->loadAssetDirectoryByName("test");
	ci::audio::BufferRef buffer = mRootBank->getRandomBuffer();



	mClock = std::make_shared<Sequencer>();
	mPlayer = ctx->makeNode( new SampleNode(ci::audio::Node::Format().channels(2)) );
	mPlayer >> ctx->getOutput();
	mPlayer->setBuffer( buffer );
	mRecorder = mAudioManager->getAudioRecorder();
	
	mLfo = ctx->makeNode( new ci::audio::GenPhasorNode() );
	mLfo->setFreq( ci::audio::master()->getSampleRate()/float(buffer->getNumFrames()) );
	
	mPlayer->getPositionParam()->setProcessor( mLfo );

	mLfo->enable();
	mPlayer->setTriggerSpeed(.05f);
	mPlayer->setLength(mPlayer->getTriggerSpeed());
	std::list<int> scale = { 0,5 };
	mPlayer->setScale(scale);
	mPlayer->enable();
	mPlayer->setVolume(.2);

	mRecorder->attachTo(mPlayer);
	mRecorder->record(false);

	mClock->start();
	ctx->enable();
}

void PlayerApp::setupMultichannelDevice()
{
	// debug print all devices to console
	console() << audio::Device::printDevicesToString() << endl;

	audio::DeviceRef deviceWithMaxOutputs;
	audio::DeviceRef deviceWithMaxInputs;
	for (const auto &dev : audio::Device::getDevices()) {
		if (!deviceWithMaxOutputs || deviceWithMaxOutputs->getNumOutputChannels() < dev->getNumOutputChannels())
			deviceWithMaxOutputs = dev;
		if (!deviceWithMaxInputs || deviceWithMaxInputs->getNumInputChannels() < dev->getNumInputChannels())
			deviceWithMaxInputs = dev;
	}

	
	console() << endl << "max output channels: " << deviceWithMaxOutputs->getNumOutputChannels() << endl;
	getWindow()->setTitle(deviceWithMaxOutputs->getName());

	auto ctx = audio::master();
	audio::OutputDeviceNodeRef multichannelOutputDeviceNode = ctx->createOutputDeviceNode(deviceWithMaxOutputs, audio::Node::Format().channels(deviceWithMaxOutputs->getNumOutputChannels()));
	mInputDeviceNode = ctx->createInputDeviceNode(deviceWithMaxInputs, audio::Node::Format().channels(deviceWithMaxInputs->getNumInputChannels()));
	ctx->setOutput(multichannelOutputDeviceNode);

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


	try {
		// Bind the receiver to the endpoint. This function may throw.
		mReceiver.bind();
	}
	catch (const osc::Exception &ex) {
		CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
		quit();
	}

	// UDP opens the socket and "listens" accepting any message from any endpoint. The listen
	// function takes an error handler for the underlying socket. Any errors that would
	// call this function are because of problems with the socket or with the remote message.
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

void PlayerApp::mouseUp(MouseEvent event)
{
	mPlayer->gate(false);
	//mPlayer->getPositionParam()->setProcessor(mLfo);
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
	mClock->update();
	inspector();
	//console() << mPlayer->getNumActiveGrains() << std::endl;
}

bool PlayerApp::inspector()
{
	if (mShowMenu) {


		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		ui::SetWindowFontScale(getDisplay()->getWidth()/1920.0f);

		float density = mPlayer->getDensity();
		if (ui::DragFloat("Density", &density, .1, 0, 100)) { mPlayer->setDensity(density); }
		ui::DragFloat("Trigger Rate", &(mPlayer->mTriggerRate), .001, 0, 1);
		ui::DragFloat("Trigger Rate Jitter", &(mPlayer->mTriggerRateJitter), .001, 0, 1);
		ui::DragFloat("Position Jitter", &(mPlayer->mPositionJitter), .001, 0, 1);
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
		

	}
	return 1;
}

void PlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	// Draw the Scope's recorded Buffer in the upper right.
	if (mInputMonitor && mInputMonitor->getNumConnectedInputs()) {
		Rectf scopeRect(getWindowWidth() - 610, 10, getWindowWidth() - 10, 510);
		drawAudioBuffer(mInputMonitor->getBuffer(), scopeRect, true);
	}
	


}

CINDER_APP( PlayerApp, RendererGl )
