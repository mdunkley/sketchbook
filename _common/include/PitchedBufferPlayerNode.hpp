//
//  Recorder.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/23/18.
//

#ifndef PitchedBufferPlayerNodeRef_hpp
#define PitchedBufferPlayerNodeRef_hpp

#include <stdio.h>
#include "cinder/audio/SamplePlayerNode.h"

typedef std::shared_ptr<class PitchedBufferPlayerNode>	PitchedBufferPlayerNodeRef;

class PitchedBufferPlayerNode : public ci::audio::BufferPlayerNode {

public:

	void setInterval(float interval) { mInterval = interval; }
	float getInterval() const { return mInterval; }
	void setRate(double r) { mRate = r; }
	float getRate() { return mRate; }

	std::atomic<float> mInterval = 0.0f;
	std::atomic<double> mRate = 1.0f;

	void seek(size_t readPositionFrames) override;

	double getReadPositionTime() const;

	void start() override;

protected:



	void process(ci::audio::Buffer *buffer) override;

private:

	float interpLinear(const float * array, size_t arraySize, float readPos);
	float interpCosine(const float * array, size_t arraySize, float readPos);
	std::atomic<double> mReadPos = 0;

	bool mHasCycled = false;
};
#endif /* PitchedBufferPlayerNodeRef_hpp */
