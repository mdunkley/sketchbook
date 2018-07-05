#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class EnvelopeFollowerNode>	EnvelopeFollowerNodeRef;

class EnvelopeFollowerNode : public ci::audio::Node
{

public:

	EnvelopeFollowerNode(const Format &format = Format());
	~EnvelopeFollowerNode();

	void setMultiplier(float mult = 1.0f) { mMultiplier = mult; }
	float getMultiplier() const { return mMultiplier; }


protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;

private:

	size_t mMaxAverage = 2500;
	size_t mAverage = 1000;
	std::array<double,128> mAccum;
	size_t mAccumCurSize = 0;
	size_t mAccumPosition = 0;
	float mMultiplier = 1.0;

	BufferDynamicRef mAccumBuffer;

};

