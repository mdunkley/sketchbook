#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class ClockNode>	ClockNodeRef;

class ClockNode : public ci::audio::Node
{

public:

	ClockNode(const Format &format = Format()) : 
		Node(format), mSync(this,0) {}
	~ClockNode();

	enum class SyncMode{ internal, external };
	enum class OutputMode { gate, ramp };

	ci::audio::Param* getSyncParam() { return &mSync; }
	
	void setMode(OutputMode m) { mMode = m; }
	OutputMode& getMode() { return mMode; }

	void setClockDivisions(int div) { mClockDivisions = std::max(1,div); }
	float getClockDivisions() const { return mClockDivisions; }

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

	OutputMode mMode = OutputMode::gate;
	size_t mTimer = 0;
	size_t mNextTick = 0;
	size_t mClockCount = 0;
	double mTickInc = 0;

	float mRate = 1.0;
	float mRateJitter = 0.0f;
	float mDutyCycle = 0.5f;
	float mDutyCycleJitter = 0.0f;
	float mDuty = 0.5;

	int mClockDivisions = 1;

	ci::audio::Param mSync;
	float* mSyncData = nullptr;
	double mSyncTime = 0;
	int mSyncCount = 0;
	double mOldSyncValue = 0;
	bool mWaitingForRisingEdge = false;

	

};

