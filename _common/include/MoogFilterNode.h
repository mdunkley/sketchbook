#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/audio/Node.h"

typedef std::shared_ptr<class MoogFilterNode>	MoogFilterNodeRef;

class MoogFilterNode : public ci::audio::Node {
public:

	MoogFilterNode(const Format &format = Format()) : Node(format) {}

	void setFrequencey(float freq) { mFreq = ci::clamp(freq,0.001f,1.0f); }
	double getFrequency() const { return mFreq; }

	void setResonance(float res) { mRes = ci::clamp(res,0.0f,4.0f); }
	double getResonance() const { return mRes; }


protected:

	void initialize()							override;
	void process(ci::audio::Buffer *buffer)		override;

	double filter(double input);

private:

	std::atomic<double> mFreq = .5;
	std::atomic<double> mRes = 1.0f;

	double mIn1 = 0.0;
	double mIn2 = 0.0;
	double mIn3 = 0.0;
	double mIn4 = 0.0;
	double mOut1 = 0.0;
	double mOut2 = 0.0;
	double mOut3 = 0.0;
	double mOut4 = 0.0;

};
