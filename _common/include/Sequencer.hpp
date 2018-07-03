//
//  Seq.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/25/18.
//

#ifndef Sequencer_hpp
#define Sequencer_hpp

#include <stdio.h>
#include <memory>
#include <cinder/Timer.h>
#include <cinder/Rand.h>
#include <cinder/Signals.h>
#include <cinder/audio/audio.h>
#include "Pattern.h"

using namespace cinder::signals;

struct StepInfo {
	long clockCount;
	int ticks;
	double stepLength;
	bool trigger;
	std::vector<double> values;
};

typedef std::shared_ptr<class Sequencer> SequencerRef;

class Sequencer : std::enable_shared_from_this<Sequencer> {

public:

	Sequencer();

	void update();
	void active(bool a);
	void start();
	void stop();
	void reset();
	
	double getTimeOffset();
	void setLength(int steps) { mLength = steps; }
	int getLength() const { return mLength; }

	void setNoteSequence(PatternRef pat) { mNoteSequence = pat; }
	PatternRef getNoteSequence() const { return mNoteSequence; }
	void clearNoteSequence() { mNoteSequence = PatternRef(); }
	void playNotes(int step);

	void setTriggerSequence(std::vector<int>& mask) { mTriggerSequence = mask; }
	std::vector<int> getTriggerSequence() const { return mTriggerSequence; }
	void clearTriggerSequence();
	void addModulationSequence(std::vector<double>& ptn);
	std::vector<double> getModulationSequence(int seq) const { return mModulationSequences.at(seq); }
	void clearModulationSequences();

	SequencerRef& clock(SequencerRef& dest);
	void setClockConnection(ci::signals::Connection connection);
	void clearClockSignals();
	void unclock();
	
	void	setClockSource(SequencerRef& src)	{ mClockSource = src; }
	void	setClockCount(int count)			{ mClockCount = count; }
	void	setIsClocked(bool clocked)			{ isClocked = clocked; }
	void	setBPM(float bpm, int ticks = 16);
	void	setRate(float rate)					{ mRate = rate; }
	float	getRate() const						{ return mRate; }
	void	setBeatOffset(int beat) 			{ mBeatOffset = beat; }
	int		getBeatOffset()	const				{ return mBeatOffset; }
	void	setRateJitter(float jitter)			{ mRateJitter = jitter; }
	float	getRateJitter() const				{ return mRateJitter; }
	void	setProbability(float prob)			{ mProbability = ci::clamp(prob, 0.0f, 1.0f); }
	float	getProbability() const				{ return mProbability;  }
	float	getStepLength() const				{ return mRate * mClockDivision; }
	void	setClockDivision(int division)		{ mClockDivision = std::max(division,1); }
	int		getClockDivision() const			{ return mClockDivision; }
	void	triggerModulation(bool t)			{ mTriggerModulation = t; }
	bool	isActive() const					{ return mActive; }
	void	setTicks(int ticks);
	int		getTicks() const { return mTicks; }
	void	trigger();
	void	trigger(StepInfo& s);

	bool isClocked = false;

	ci::signals::Signal<void(StepInfo)>&	getClockSignal()	{ return mClockSig; }
	ci::signals::Signal<void(StepInfo)>&	getTriggerSignal()	{ return mTriggerSig; }
	ci::signals::Signal<void(NoteEvent)>&   getNoteSignal()		{ return mNoteSig; }
	ci::signals::Signal<void()>&			getResetSignal()	{ return mResetSig;	}

	int		mClockDivision = 1;
	float	mProbability = 1;
	int		mLength = 0;
	bool	mTriggerModulation = false;
	int		mBeatOffset = 0;

private:


	void runStep();
	int wrap(int kX, int const kLowerBound, int const kUpperBound);

	std::vector<SequencerRef> mChildren;

	std::vector<double> collectModulations();
	std::vector<ci::signals::Connection> mConnections;
	std::vector<int> mTriggerSequence;
	std::vector< std::vector<double> > mModulationSequences;
	PatternRef mNoteSequence;

	cinder::signals::Connection mClockSourceConnection;
	SequencerRef mClockSource;
	SequencerRef mThis;
    
    cinder::signals::Signal<void(StepInfo)>		mClockSig;
	cinder::signals::Signal<void(StepInfo)>		mTriggerSig;
	cinder::signals::Signal<void(NoteEvent)>	mNoteSig;
	cinder::signals::Signal<void()>				mResetSig;

	bool	mLookAheadEnabled = true;
	float	mLookAhead = 0.03f;
	double	mNextNoteTime = 0.0;
	bool	mActive = false;
	int		mClockCount = 0;
	int		mClockDivisionCount = 0;
	int		mTriggerCount = 0;
    float	mRate = 0.0f;
    float	mRateJitter = 0.0f;
    float	mRateJitterVal = 0.0f;;
    double	mPrevClock = -1.0;
    int		mTickCount = 0; 


	int		mTicks = 16;

};

#endif /* Sequencer_hpp */
