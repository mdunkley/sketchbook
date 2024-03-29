#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class ComparatorNode>	ComparatorNodeRef;

class ComparatorNode : public ci::audio::Node
{

public:

	ComparatorNode(const Format &format = Format());
	~ComparatorNode();

	void setThreshold(float thresh) { mThreshold.setValue(thresh); }
	float getThreshold() { return mThreshold.getValue(); }

protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;

private:

	ci::audio::Param mThreshold;

};

