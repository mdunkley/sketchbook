#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/GenNode.h"
#include "osc.h"
#include "cinder/audio/GainNode.h"
#include "CinderImGui.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "AudioDrawUtils.h"
#include "EnvelopeFollowerNode.h"
#include "ComparatorNode.h"
#include "ClockNode.h"
#include "TunerNode.h"

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
	void mouseUp(MouseEvent event) override;
	void mouseDown( MouseEvent event ) override;
	void mouseMove(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void keyDown(KeyEvent event);
	void update() override;
	bool inspector();
	void draw() override;


	ci::audio::InputDeviceNodeRef mInputDeviceNode;
	ci::audio::MonitorNodeRef	mInputMonitor;

	EnvelopeFollowerNodeRef mEnvelopeFollowerNode;
	ci::audio::MonitorNodeRef mAverageMonitor;
	ComparatorNodeRef mComparator;
	ClockNodeRef mMasterClock;
	ClockNodeRef mClockNode;
	TunerNodeRef mTuner;
	GenSineNodeRef mSine;
	GenTriangleNodeRef mTriangle;

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

	mInputMonitor = ctx->makeNode(new ci::audio::MonitorNode());
	mAverageMonitor = ctx->makeNode(new ci::audio::MonitorNode());

	mEnvelopeFollowerNode = ctx->makeNode(
		new EnvelopeFollowerNode(ci::audio::Node::Format().channels(2).
		channelMode(ci::audio::Node::ChannelMode::MATCHES_INPUT)));
	mEnvelopeFollowerNode->setMultiplier(5);
	mComparator = ctx->makeNode(
		new ComparatorNode(ci::audio::Node::Format().channels(2)));
	mClockNode = ctx->makeNode(new ClockNode());
	mMasterClock = ctx->makeNode(new ClockNode());
	mSine = ctx->makeNode(new GenSineNode());
	mTriangle = ctx->makeNode(new GenTriangleNode());
	mTuner = ctx->makeNode(new TunerNode());

	mClockNode->enable();
	mClockNode->setRate( 5 );
	mClockNode->setMode( ClockNode::OutputMode::ramp );

	mMasterClock->enable();
	mMasterClock->setRate(.2);
	mClockNode->getSyncParam()->setProcessor(mMasterClock);
	mTriangle->setFreq(200);
	mSine->setFreq(200);
	mTriangle >> mTuner;
	mTriangle->getParamFreq()->setProcessor(mTuner);
	//mSine >> ctx->getOutput();
	mTriangle >> mAverageMonitor;
	mTriangle >> ctx->getOutput();
	mAverageMonitor->enable();

	mSine->enable();
	mTuner->enable();
	mTriangle->enable();
	ctx->enable();

	setupOSC();
	
}

void PlayerApp::setupAudioDevice()
{

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

	});

	mReceiver.setListener("/4/toggle1",
		[&](const osc::Message &msg) {

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
	//mSine->setFreq(ci::audio::midiToFreq(std::floor(relX * 127)));
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
		break;

	case 'n':
		break;

	}
}

void PlayerApp::update()
{
	//mClock->update();
	inspector();

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

	}
	return 1;
}

void PlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	if (mEnvelopeFollowerNode &&
		mEnvelopeFollowerNode->getNumChannels() > 0 &&
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
