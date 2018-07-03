
#include "PitchedBufferPlayerNode.hpp"
#include "cinder/audio/audio.h"
#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/Log.h"


using namespace ci;
using namespace std; 
using namespace ci::log;



inline float PitchedBufferPlayerNode::interpLinear(const float *array, size_t arraySize, float readPos)
{
	size_t index1 = (size_t)readPos;
	size_t index2 = (index1 + 1) % arraySize;
	float val1 = array[index1];
	float val2 = array[index2];
	float frac = readPos - (float)index1;

	return val2 + frac * (val2 - val1);
}

inline float PitchedBufferPlayerNode::interpCosine(const float *array, size_t arraySize, float readPos)
{
	size_t index1 = (size_t)readPos;
	size_t index2 = (index1 + 1) % arraySize;
	float val1 = array[index1];
	float val2 = array[index2];
	float frac = readPos - (float)index1;

	float mu2 = (1 - cos(frac*M_PI)) / 2;
	return(val1*(1 - mu2) + val2 * mu2);
}


void PitchedBufferPlayerNode::process(ci::audio::Buffer * buffer)
{

	// TODO set up to work with loop
	if (isEnabled()) {

		const auto &frameRange = getProcessFramesRange();

		double readPos = mReadPos;
		size_t readCount = 0;
		size_t numFrames = frameRange.second - frameRange.first;
		size_t readEnd = mLoop ? mLoopEnd.load() : mNumFrames;
		size_t writePos = frameRange.first;
		size_t srcFrameCount = mBuffer->getNumFrames();
		size_t dstFrameCount = buffer->getNumFrames();
		size_t maxFrame = mLoop ? size_t(mLoopEnd) : srcFrameCount;
		size_t minFrame = mLoop ? size_t(mLoopBegin) : 0;

		float transRatio = exp(.057762265 * mInterval);
		int srcChannels = mBuffer->getNumChannels();
		int dstChannels = buffer->getNumChannels();

		int channels = std::max(buffer->getNumChannels(), mBuffer->getNumChannels());

		while (readCount < numFrames) {

			readPos += mRate * transRatio;
			if (readPos >= maxFrame) {
				if (mLoop) {
					readPos = mLoopBegin;
				}
				else {
					mIsEof = true;
					mReadPos = mNumFrames;
					stop();
					disable();
					break;
				}
			}
			else if (readPos < minFrame) {
				if( mLoop ) readPos = mLoop ? size_t(mLoopEnd) : srcFrameCount;
				else {
					mIsEof = true;
					mReadPos = minFrame;
					stop();
					disable();
					break;
				}
			}

			int offset = writePos + readCount;

			for (int ch = 0; ch < channels; ch++) {

				float value = interpCosine(mBuffer->getData(), mBuffer->getSize(), ch * srcFrameCount + readPos);
				(*buffer)[offset + ch * dstFrameCount] = value;
			}

			readCount++;
		}

		mReadPos.store(readPos);
	}

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

void PitchedBufferPlayerNode::seek(size_t readPositionFrames)
{
	mIsEof = false;
	mReadPos = math<size_t>::clamp(readPositionFrames, 0, mNumFrames);
}

double PitchedBufferPlayerNode::getReadPositionTime() const
{
	return (double)mReadPos / (double)getSampleRate();
}

void PitchedBufferPlayerNode::start()
{
	float minFrame = mLoop ? getLoopBegin() : 0;
	if (mRate < 0 && mReadPos <= minFrame ) mReadPos = mLoop ? getLoopEnd() : mBuffer->getNumFrames();
	enable();
}


