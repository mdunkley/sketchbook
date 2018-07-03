#pragma once

#include <stdio.h>
#include <array>
#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/audio/Node.h"

typedef std::shared_ptr<class AmbienceNode>	AmbienceNodeRef;

class AmbienceNode : public ci::audio::Node {
public:

	AmbienceNode(const Format &format = Format()) : Node(format) {}

	void setFrequencey(float freq) { mFreq = ci::clamp(freq,0.001f,1.0f); }
	double getFrequency() const { return mFreq; }

	void setResonance(float res) { mRes = ci::clamp(res,0.0f,4.0f); }
	double getResonance() const { return mRes; }


protected:

	void initialize()							override;
	void process(ci::audio::Buffer *buffer)		override;

	void setParameters();

	double reverb(double input, int ch);

	void suspend();


private:

	std::atomic<double> mSize = .7;
	std::atomic<double> mHighPassFilter = 0.7f;
	std::atomic<double> mMix = .9f;
	std::atomic<double> mOutput = .5f;

	std::atomic<float> fil, fbak, damp, wet, dry, size;
	size_t  pos, den, rdy;

	float mBuf1[1024];
	float mBuf2[1024];
	float mBuf3[1024];
	float mBuf4[1024];


};
