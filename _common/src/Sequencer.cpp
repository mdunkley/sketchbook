//
//  Sequencer.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/25/18.
//

#include "Sequencer.hpp"
#include "CommonUtils.h"

#include "cinder/app/App.h"
#include "cinder/Rand.h"


Sequencer::Sequencer(){

	clearModulationSequences();
	clearTriggerSequence();
	mRateJitterVal = 0.0;
	
}

void Sequencer::update(){
   
	// Update internal timer
    float timeOffset = getTimeOffset();

	if (mActive && mLookAheadEnabled) {
		double audioClock = ci::audio::master()->getNumProcessedSeconds();
		if (mNextNoteTime - audioClock < (double)mLookAhead) {

			mNextNoteTime += timeOffset;
			if (!isClocked) {
				runStep();
			}

		}
	}

}

double Sequencer::getTimeOffset() {
	return  mRate * mClockDivision + mRateJitterVal * mRateJitter;

}

void Sequencer::active( bool a ){

    if( a ) start();
    else stop(); 

}

void Sequencer::start(){
	mNextNoteTime = ci::audio::master()->getNumProcessedSeconds();
	mActive = true;
	reset();
}

void Sequencer::stop() {
	mActive = false;
}

void Sequencer::addModulationSequence(std::vector<double>& ptn) {
	mModulationSequences.push_back(ptn);
}

void Sequencer::clearTriggerSequence() {
	mTriggerSequence.clear();
	mTriggerSequence.push_back(1);
}

void Sequencer::clearModulationSequences() {
	mModulationSequences.clear();
}

SequencerRef& Sequencer::clock(SequencerRef& dest) {

	ci::app::console() << "Setting up clock" << std::endl;

	dest->setTicks(mTicks);
	dest->setRate(mRate*mClockDivision);
	dest->setIsClocked(true);
	dest->setClockCount(mClockCount);
	dest->start();

	mChildren.push_back(dest);

	ci::signals::Connection connect = getClockSignal().connect([&](StepInfo s) {dest->trigger( s );});
	dest->setClockConnection(connect);
	
	return dest;

}

void Sequencer::setClockConnection(ci::signals::Connection connection) {
	 mClockSourceConnection.disconnect();
	 mClockSourceConnection = connection;
}

void Sequencer::clearClockSignals()
{
	for (auto& c : mConnections) {
		c.disconnect();
	}
	mConnections.clear();
}



void Sequencer::unclock() {
	
	mClockSourceConnection.disconnect();
}

void Sequencer::reset( ) {

	mClockCount = 0;
	mClockDivisionCount = 0;
	mTriggerCount = 0;
	mResetSig.emit();

	for (auto& child : mChildren) {
		child->reset();
	}
}

void Sequencer::setBPM(float bpm, int ticks) {

	mTicks = ticks;
	mRate = 60 / bpm / mTicks;

	for (auto& child : mChildren) {
		child->setBPM(bpm, mTicks);
	}
}

void Sequencer::runStep() {
	if (mActive) {
		if (mClockCount%mClockDivision == 0) {

			// Sample the trigger sequence
			bool trig = false;
			if (mTriggerSequence.size())
				trig = mTriggerSequence[(mClockDivisionCount - mBeatOffset) % mTriggerSequence.size()] > 0;


			// Sample each sequence in the modulation stack
			std::vector<double> stepValues = collectModulations();

			// Build the step clock info
			StepInfo step{
				mClockDivisionCount,
				mTicks,
				mRate*mClockDivision,
				trig,
				stepValues,
			};

			// Clock signal
			mClockSig.emit(step);

			if (trig &&  ci::Rand::randFloat() <= mProbability) {

				// Build the step trigger info
				StepInfo t{
					mTriggerCount,
					mTicks,
					mRate*mClockDivision,
					trig,
					stepValues,
				};

				// Trigger signal
				mTriggerSig.emit(t);
				mTriggerCount++;
			}

			if (ci::Rand::randFloat() <= mProbability) {
				int pos = mClockDivisionCount - mBeatOffset;
				playNotes(pos);
			}

			mClockDivisionCount++;
			if (mLength > 0 && mClockDivisionCount >= mLength) reset();
		}

		mRateJitterVal = ci::Rand::randFloat();

		if (!isClocked) mClockCount++;
	}

}

void Sequencer::setTicks(int ticks) {
	mTicks = ticks;
	for (auto& child : mChildren) {
		child->setTicks(mTicks);
	}
}

void Sequencer::playNotes(int step) {

	if (mNoteSequence && mNoteSequence->hasNote(step)) {
		for (auto& n : mNoteSequence->getStep(step)) {
			if (ci::Rand::randFloat() <= mProbability) mNoteSig.emit(n);
		}
	}
		
}

std::vector<double> Sequencer::collectModulations() {

	int numSequences = mModulationSequences.size();
	std::vector<double> stepValues;
	for (int i = 0; i < numSequences; i++) {

		std::vector<double> mseq = mModulationSequences.at(i);
		if (mseq.size() > 0) {
			int drive = mClockDivisionCount;
			if (mTriggerModulation) drive = mTriggerCount;
			stepValues.push_back(mseq.at(drive % mseq.size()));
		}
		else stepValues.push_back(0.0);
	}

	return stepValues;

}

void Sequencer::trigger(){

	isClocked = true;
	runStep();
}

void Sequencer::trigger(StepInfo& s) {

	if (!isClocked) setIsClocked(true);

	setRate(s.stepLength);
	setClockCount(s.clockCount);
	
	runStep();
}

