

#pragma once

#include <stdio.h>
#include <memory>
#include <cinder/app/App.h>
#include <cinder/Timer.h>
#include <cinder/Rand.h>
#include <cinder/Signals.h>


struct NoteEvent {
	int note;
	float duration;
	float velocity;
};

typedef std::shared_ptr<class Pattern> PatternRef;
typedef std::shared_ptr<class PatternBank> PatternBankRef;

class Pattern : std::enable_shared_from_this<Pattern>{

public:

	Pattern();
	~Pattern();

	void	setClockDiv(int div) { mClockDiv = div; }
	int		getClockDiv() const { return mClockDiv;  }
	void	setLength(int length) { mLength = length; }
	int		getLength() const { return mLength; }

	std::vector<NoteEvent>& getStep(int step);
	bool					hasNote(int step);

	PatternRef getRef() { return shared_from_this(); }

	static PatternRef patternFromDivisions(int divisions, float length, float beats, int ticks = 16);
	static PatternRef patternFromSlices(std::vector<float> slices, float length, float beats, int ticks = 16);

	void setNotes(std::map<int, std::vector<NoteEvent>> notes) { mNotes = notes; }
	std::map<int, std::vector<NoteEvent>> getNoteSequence() const { return mNotes; }
	void addNotes(int step, std::vector<NoteEvent>& events);
	void addNote(int step, NoteEvent event);
	void clear() { mNotes.clear(); }
	bool isEmpty() { return mNotes.size()==0; }

	void print();

private:
	std::vector<NoteEvent> mEmptyStep;
	std::map<int, std::vector<NoteEvent>> mNotes;
	int mClockDiv = 1;
	int mLength = 0;
	
};

class PatternBank {
public:
	PatternBank();
	~PatternBank();

	void addPattern(int index, PatternRef pat);
	void addPattern(PatternRef pat);
	PatternRef getPattern(int pat);
	
	size_t size() const { return mPatterns.size(); }

private:

	std::map<int, PatternRef> mPatterns;
	PatternRef mEmptyPattern;



};

