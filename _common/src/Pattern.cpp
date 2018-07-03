#include "Pattern.h"

Pattern::Pattern()
{
}

Pattern::~Pattern()
{
}

void Pattern::addNote(int step, NoteEvent event) {

	auto it = mNotes.find(step);
	
	if (it != mNotes.end()) {
		it->second.push_back( event );
	} 
	else {
		std::vector<NoteEvent> n({ event });
		mNotes.insert( std::pair<int, std::vector<NoteEvent>>(step, n) );
	}

}

void Pattern::addNotes(int step, std::vector<NoteEvent>& events) {

	for (auto& event : events) {
		addNote(step, event);
	}

}

std::vector<NoteEvent>& Pattern::getStep(int step) {

	//ci::app::console() << "STEP REQUESTED " << step << std::endl;
	if (!mNotes.empty()) {
		std::map<int, std::vector<NoteEvent>>::iterator it;
		it = mNotes.find(step);
		if (it != mNotes.end()) {
			//ci::app::console() << (it->second)[0].note << " " << (it->second)[0].duration << " " << (it->second)[0].velocity << std::endl;
			return it->second ;
		}
		else return mEmptyStep;
	} else return mEmptyStep;

}

bool Pattern::hasNote(int step) {

	if (!mNotes.empty()) {
		std::map<int, std::vector<NoteEvent>>::iterator it;
		it = mNotes.find(step);
		return it != mNotes.end();
	}
	else return false;

}

PatternRef Pattern::patternFromDivisions(int divisions, float length, float beats, int ticks )
{
	PatternRef pat = std::make_shared<Pattern>();
	int patternTicks = beats * ticks;
	for (int i = 0; i < divisions; i++) {

		int step = std::floor(patternTicks/divisions) * i ;
		NoteEvent note{	i, 	length / divisions,	100	};
		pat->addNote(step, note);
	}

	pat->setLength(beats*ticks);

	return pat;
}

PatternRef Pattern::patternFromSlices(std::vector<float> slices, float length, float beats, int ticks)
{
	PatternRef pat = std::make_shared<Pattern>();
	int patternTicks = beats * ticks;

	int count = 0;
	for (auto& s : slices) {
		int step, nextstep= 0;
		float timestart, timeend = 0;
		step = ci::clamp(int(std::floor(patternTicks*s + .5)), 0, int(slices.size()));  // rounded step
		timestart = float(step) / patternTicks * length;
		if ((count + 1) == slices.size()) {
			timeend = length;
		}
		else {
			nextstep = ci::clamp(int(std::floor(patternTicks*s + 1.5)), 0, int(slices.size()));
			timeend = float(nextstep) / patternTicks * length;
		}
		NoteEvent note{ count, timeend - timestart, 100 };
		pat->addNote(step, note);
		count++;
	}

	return pat;
}


void Pattern::print() {

	ci::app::console() << "Pattern ";
	for (std::map<int, std::vector<NoteEvent>>::iterator it = mNotes.begin(); it != mNotes.end(); ++it) {
		for (auto& n : it->second) {
			//ci::app::console() << n.note << " " << n.duration << " " << n.velocity << " ";
			ci::app::console() << n.note << " ";
		}
	}
	ci::app::console() << std::endl;

}


PatternBank::PatternBank() {
	mEmptyPattern = std::make_shared<Pattern>();
}

PatternBank::~PatternBank() {

}

void PatternBank::addPattern(int index, PatternRef pat) {
	mPatterns[index] = pat;
}

void PatternBank::addPattern(PatternRef pat) {
	int index = mPatterns.size();
	mPatterns[index] = pat;
}

PatternRef PatternBank::getPattern( int pat ) {
	if (!mPatterns.empty()) {
		std::map<int, PatternRef>::iterator it;
		it = mPatterns.find(pat);
		if (it != mPatterns.end()) {
			return it->second;
		}
		else return mEmptyPattern;
		
	}
	else return mEmptyPattern;
}