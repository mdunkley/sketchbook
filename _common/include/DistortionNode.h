#pragma once

#include "cinder/Cinder.h"
#include "cinder/audio/Node.h"

typedef std::shared_ptr<class DistortionNode>	DistortionNodeRef;

class DistortionNode : public ci::audio::Node {
public:

	DistortionNode(const Format &format = Format()) : Node(format) {}

	void setAmplify(float mult) { mAmp = mult; }
	float getAmplify() const { return mAmp; }

	void setThreshold(float thresh) { mThreshold = thresh; }
	float getThreshold() const { return mThreshold; }

	void setBitRateMult(float mult) { mBitRateMult = mult; }
	void setSampleRateMult(float mult) { mSampleRateMult = mult; }

protected:

	void initialize()							override;
	void process(ci::audio::Buffer *buffer)		override;

	double distort(double x);

private:

	std::atomic<float> mAmp = 1;
	std::atomic<float> mThreshold = 0.5;
	std::atomic<float> mSampleRateMult = 0.0;
	std::atomic<float> mBitRateMult = 0.0;


	std::atomic<size_t> mCount = 0;
	float mChannelValues[8];

	size_t mSampleRate = 44100;
};
