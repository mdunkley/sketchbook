#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"
#include "ClockNode.h"
#include <thread>
#include "CommonUtils.h"
#include "Circuits.h"

using namespace cinder::audio;

struct SequencerStep {
	double value;
	int rachet;
};


typedef std::shared_ptr<class SequencerNode>	SequencerNodeRef;

class SequencerNode : public ci::audio::Node
{

public:

	SequencerNode(const Format &format = Format());
	~SequencerNode();

	void draw();

	void setClockDivision(size_t divs) { mClockDivisions = std::max((size_t)1,divs); }
	size_t getClockDivision() const { return mClockDivisions; }

	void setDelaySize(size_t samples);
	size_t getDelaySize() { return mDelaySize; }
	

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

	Circuits::DelayRef mSigDelay;
	Circuits::RisingEdgeTrigger mTrigDetect;
	Circuits::Change mChange;

	size_t mTriggerCount = 0;
	size_t mClockDivisions = 1;
	size_t mDelaySize = 0;

	ci::audio::Param mPosition;

};

