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


class PlayerApp : public App {
  public:

	  PlayerApp();

	void setup() override;
	void setupAudioDevice();

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


	ci::audio::MonitorNodeRef mAverageMonitor;
	TunerNodeRef mTuner;
	GenSineNodeRef mSine;
	GenTriangleNodeRef mTriangle;

	bool mShowMenu = true;
};

const uint16_t localPort = 4000;

PlayerApp::PlayerApp()
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

	mSine = ctx->makeNode(new GenSineNode());
	mTriangle = ctx->makeNode(new GenTriangleNode());
	mTuner = ctx->makeNode(new TunerNode());

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

void PlayerApp::mouseUp(MouseEvent event)
{

}

void PlayerApp::mouseDown( MouseEvent event )
{
	mTuner->calibrate();

}

void PlayerApp::mouseMove(MouseEvent event) {

	float relX = ci::clamp( event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1-(event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);

}

void PlayerApp::mouseDrag(MouseEvent event) {

	float relX = ci::clamp(event.getX() / float(app::getWindowWidth()), 0.0f, 1.0f);
	float relY = ci::clamp(1 - (event.getY() / float(app::getWindowHeight())), 0.0f, 1.0f);

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
	inspector();
}

bool PlayerApp::inspector()
{
	mShowMenu = false;
	if (mShowMenu) {

		ui::ScopedWindow window("Setup (Spacebar to toggle)");
		ui::SetWindowFontScale(getDisplay()->getWidth()/1920.0f);
	}
	return 1;
}

void PlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	if (mAverageMonitor &&
		mAverageMonitor->getNumConnectedInputs()) {
		Rectf scopeRect(getWindowWidth() - 610, 10, getWindowWidth() - 10, 100);
		drawAudioBuffer(mAverageMonitor->getBuffer(), scopeRect, true);
	}
	
}

CINDER_APP( PlayerApp, RendererGl )
