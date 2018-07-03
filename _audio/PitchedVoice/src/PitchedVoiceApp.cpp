#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"
#include "cinder/audio/SamplePlayerNode.h"
#include "PitchedBufferPlayerNode.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::log;

namespace cinder {
	namespace audio {

		namespace {

			inline float interpLinear(const float *array, size_t arraySize, float readPos)
			{
				size_t index1 = (size_t)readPos;
				size_t index2 = (index1 + 1) % arraySize;
				float val1 = array[index1];
				float val2 = array[index2];
				float frac = readPos - (float)index1;

				return val2 + frac * (val2 - val1);
			}

			inline float interpCosine(const float *array, size_t arraySize, float readPos)
			{
				size_t index1 = (size_t)readPos;
				size_t index2 = (index1 + 1) % arraySize;
				float val1 = array[index1];
				float val2 = array[index2];
				float frac = readPos - (float)index1;

				float mu2 = (1 - cos(frac*M_PI)) / 2;
				return(val1*(1 - mu2) + val2 * mu2);
				//return val2 + frac * (val2 - val1);
			}

		} // anonymous namespace
	}
}

class PitchedBufferPlayer : public audio::BufferPlayerNode {
public:
	void setInterval(float interval) { mInterval = interval; }
	float getInterval() const { return mInterval; }
	void setRate(double r) { mRate = r; }
	float getRate() { return mRate; }
protected:
	void process(ci::audio::Buffer *buffer) override;
private:
	std::atomic<float> mInterval = 0;
	std::atomic<double> mRate = 1.0f;
	std::atomic<double> mReadPos = 0;
};

void PitchedBufferPlayer::process(ci::audio::Buffer * buffer)
{
	const auto &frameRange = getProcessFramesRange();

	double readPos = mReadPos;
	size_t readCount = 0;
	size_t numFrames = frameRange.second - frameRange.first;
	size_t readEnd = mLoop ? mLoopEnd.load() : mNumFrames;
	size_t writePos = frameRange.first;
	size_t srcFrameCount = mBuffer->getNumFrames();
	size_t dstFrameCount = buffer->getNumFrames();
	size_t channelOffset = 0;
	float transRatio = exp(.057762265 * mInterval);
	int srcChannels = mBuffer->getNumChannels();
	int dstChannels = buffer->getNumChannels();

	int channels = std::max(buffer->getNumChannels(), mBuffer->getNumChannels());

	while (readCount < numFrames) {

		readPos += mRate*transRatio;
		if (readPos >= srcFrameCount) readPos -= srcFrameCount;
		else if (readPos < 0) readPos += srcFrameCount;

		int offset = writePos + readCount;
		offset = offset >= dstFrameCount ? offset - dstFrameCount : offset;

		for (int ch = 0; ch < channels; ch++) {
			float value = audio::interpCosine(mBuffer->getData(), mBuffer->getSize(), ch * srcFrameCount + readPos);
			(*buffer)[offset + ch * dstFrameCount] = value;
		}

		readCount++;
	}

	mReadPos = readPos;

	/*
	if (readPos <= readEnd) {
		readCount = min(readEnd - readPos, double(numFrames));
		buffer->copyOffset(*mBuffer, readCount, frameRange.first, readPos);
	}


	//buffer[writePos] = ci::audio::interpLinear(mBuffer->getData(), mBuffer->getSize(), .1);

	if (readCount < numFrames) {
		// End of File. If looping copy from beginning, otherwise disable and mark mIsEof.
		if (mLoop) {
			size_t readBegin = mLoopBegin;
			size_t readLeft = min(numFrames - readCount, mNumFrames - readBegin);

			buffer->copyOffset(*mBuffer, readLeft, readCount, readBegin);
			mReadPos.store(readBegin + readLeft);
		}
		else {
			mIsEof = true;
			mReadPos = mNumFrames;
			disable();
		}
	}
	else
		mReadPos += readCount;
	*/
}


class PitchedVoiceApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;


	PitchedBufferPlayerNodeRef mPlayer;
	ci::audio::BufferRef mBuffer;

};

void PitchedVoiceApp::setup()
{
	auto ctx = audio::master();
	mPlayer = ctx->makeNode(new PitchedBufferPlayerNode);

	// create a SourceFile and set its output samplerate to match the Context.
	fs::path path = fs::canonical( ci::app::getAssetDirectories()[0] / "../../../common/audio/beauford/splits/synth2_4.wav" ) ;
	if (fs::exists(path)) {
		audio::SourceFileRef sourceFile = audio::load(ci::loadFile(path), ctx->getSampleRate());

		mPlayer->loadBuffer(sourceFile);
	}

	mPlayer->setLoopEnabled(false);

	mPlayer>>ctx->getOutput();

	mPlayer->enable();
	ctx->enable();
}

void PitchedVoiceApp::mouseDown( MouseEvent event )
{
	if (event.isShiftDown()) {
		float relY = std::floor((1 - ((event.getY() / float(app::getWindowHeight()))) - .5) * 48 + .5);
		mPlayer->setInterval(relY);
		ci::app::console() << "Interval Offset: " << relY << std::endl;

	}
	else if( event.isControlDown() ) {
		float relX = 2 * ((event.getX() / float(app::getWindowWidth())) - .5);
		mPlayer->setRate(relX);
		ci::app::console() << "Rate: " << relX << std::endl;
	}
	else {
		float relX = event.getX() / float(app::getWindowWidth());
		mPlayer->stop();
		mPlayer->start();
		mPlayer->seek(relX * mPlayer->getBuffer()->getNumFrames());
	}

}

void PitchedVoiceApp::update()
{
}

void PitchedVoiceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}



CINDER_APP( PitchedVoiceApp, RendererGl )
