#pragma once

#include "cinder/audio/audio.h"
#include "Circuits.h"

typedef std::shared_ptr<class TestNode>	TestNodeRef;

class TestNode : public ci::audio::Node
{
public:
	TestNode(const Format &format = Format());
	~TestNode();

protected:
	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;

	Circuits::AllPassFilterRef mAllPass;
};

