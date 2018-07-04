#pragma once

#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class AverageNode>	AverageNodeRef;

class AverageNode : public ci::audio::Node
{

public:

	AverageNode(const Format &format = Format());
	~AverageNode();

protected:


	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;

private:

	size_t mMaxAverage = 48000;
	size_t mAverage = 2000;
	double mAccum = 0.0;
	size_t mAccumCurSize = 0;
	size_t mAccumPosition = 0;

	BufferDynamicRef mAccumBuffer;

};

