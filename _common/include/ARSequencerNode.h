#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

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

	ci::audio::Param mThreshold;

};

