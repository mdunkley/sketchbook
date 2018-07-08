#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class ClockNode>	ClockNodeRef;

class ClockNode : public ci::audio::Node
{

public:

	ClockNode(const Format &format = Format());
	~ClockNode();

	void setRate(float rate) { mRate = rate; }
	float getRate() { return mRate; }
	void setRateJitter(float rate) { mRateJitter = rate; }
	float getRateJitter() { return mRateJitter; }
	void setDutyCycle(float duty) { mDutyCycle = duty; }
	float getDutyCycle() { return mDutyCycle; }
	void setDutyCycleJitter(float duty) { mDutyCycleJitter = duty; }
	float getDutyCycleJitter() { return mDutyCycleJitter; }

protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;
	virtual void tick();

private:

	size_t mTimer = 0;
	size_t mNextTick = 0;
	double mTickInc = 0;

	float mRate = 1.0;
	float mRateJitter = 0.0f;
	float mDutyCycle = 0.5f;
	float mDutyCycleJitter = 0.0f;
	float mDuty = 0.5;
	

};

