#pragma once

#include <list>
#include <array>
#include <stdio.h>
#include "cinder/CinderMath.h"
#include "cinder/audio/audio.h"
#include "cinder/Log.h"

using namespace cinder::log;




namespace Circuits {

	typedef std::shared_ptr<class SampleAndHold> SampleAndHoldRef;
	typedef std::shared_ptr<class Delay> DelayRef;
	typedef std::shared_ptr<class History> HistoryRef;
	typedef std::shared_ptr<class Delta> DeltaRef;
	typedef std::shared_ptr<class Change> ChangeRef;
	typedef std::shared_ptr<class RisingEdgeTrigger> RisingEdgeTriggerRef;

	class SampleAndHold {
	public:
		float process(float invalue, float gatesig);
	private:
		float mValue = 0;
	};

	class Delay {

	public:

		Delay();
		~Delay();

		void setDelaySize(size_t samples);
		size_t getDelaySize() { return mDelaySize; }
		float process(float invalue, size_t samples);
		float process(float invalue);

	private:
		
		//float *arr;
		std::vector<float> mDelayLine;
		std::atomic<size_t> mDelaySize = 48000;
		std::atomic<size_t> mDelayMax = 48000;
		long mReadHead = 0;
		long mWriteHead = 0;

	};

	class History {
	public:
		float process(float value);
	private:
		float mPreviousValue = 0;
	};

	class Delta {
	public:
		float process(float value) {
			float outvalue = value - mPreviousValue;
			mPreviousValue = value;
			return outvalue;
		}
	private:
		float mPreviousValue = 0;
	};

	class Change {
	public:
		float process(float value) {
			float outvalue = 0;
			if (value > mPreviousValue) outvalue = 1;
			else if (value < mPreviousValue) outvalue = -1;
			mPreviousValue = value;
			return outvalue;
		}
	private:
		float mPreviousValue = 0;
	};

	class RisingEdgeTrigger {
	public:

		enum class State { wait, arm, active };
		float process(float value, float gatelength = 0);

	private:

		State mState = State::wait;
		double mEnvInc;
		double mTimer;
		float mPrevValue;
		double mPrevTimer = 0;
	};

	class AREnvelope {

	public:
		
		float operator()(float triggerSig);
		float operator()(float triggerSig, float attack, float decay);
		enum class State { wait, attack, decay };

		void setAttack(size_t size) { mAttackLength = size; }
		size_t getAttack() const { return mAttackLength; }

		void setDecay(size_t size) { mDecayLength = size; }
		size_t getDecay() const { return mDecayLength; }

	private:

		float mValue = 0;
		State mState = State::wait;
		float mAttackLength = 1024;
		float mDecayLength = 1024;
		double mInc = 0;

	};
}