#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"
#include "ARClockNode.h"
#include <thread>
#include "CommonUtils.h"

using namespace cinder::audio;

struct ARSequencerStep {
	double value;
	int rachet;
};


typedef std::shared_ptr<class ARSequencerNode>	ARSequencerNodeRef;

class ARSequencerNode : public ci::audio::Node
{

public:

	ARSequencerNode(const Format &format = Format());
	~ARSequencerNode();

	void setClockDivision(size_t divs) { mClockDivisions = std::max((size_t)1,divs); }
	size_t getClockDivision() const { return mClockDivisions; }

	void setSequence(std::vector<float> values);

	enum class Direction { up, down, updown, walk, random };

protected:

	void	initialize()							override;
	void	getNextStep();
	void	process(ci::audio::Buffer *buffer)		override;

private:

	std::array<float, 128> mSequence;
	Direction mDirection = Direction::up;
	
	std::atomic<int> mLength = 1;
	std::atomic<int> mCurrentStep = 0;

	AudioOp::TriggerDetect mTrigDetect;
	size_t mTriggerCount = 0;
	size_t mClockDivisions = 1;

	ci::audio::Param mPosition;

};

