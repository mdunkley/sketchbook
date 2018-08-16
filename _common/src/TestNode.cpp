#include "TestNode.h"
#include <stdio.h>


using namespace ci::audio;

TestNode::TestNode(const Format &format) :
	Node(format)
{
	;
}

TestNode::~TestNode() {
	
}

void TestNode::initialize()
{
	mAllPass = std::make_shared<Circuits::AllPassFilter>();
}

void TestNode::process( ci::audio::Buffer * buffer )
{
	const int numChannels = buffer->getNumChannels();
	const auto &frameRange = getProcessFramesRange();
	const size_t bufferFrames = frameRange.second - frameRange.first;
	float *bufferData = buffer->getData();

	size_t readCount = 0;
	while (readCount < bufferFrames) {
		bufferData[readCount] = (*mAllPass)(bufferData[readCount]);
		readCount++;
	}
}
