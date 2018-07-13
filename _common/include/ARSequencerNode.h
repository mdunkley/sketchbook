#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"
#include "ARClockNode.h"
#include <thread>
#include "CommonUtils.h"

using namespace cinder::audio;

typedef std::shared_ptr<class ARSequencerNode>	ARSequencerNodeRef;

class ARSequencerNode : public ci::audio::Node
{

public:

	ARSequencerNode(const Format &format = Format());
	~ARSequencerNode();


protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;

private:

	//std::atomic<std::vector<float>> mValues;

	AudioOp::TriggerDetect mTrigDetect;
	ci::audio::Param mThreshold;
	ci::audio::Param mPosition;

};

