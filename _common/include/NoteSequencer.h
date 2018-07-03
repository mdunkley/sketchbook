#pragma once

#include <stdio.h>
#include "Sequencer.hpp"
#include "cinder/Filesystem.h"


class NoteSequencer : Sequencer
{
public:
	NoteSequencer();
	~NoteSequencer();

	void loadSequenceFromColl( ci::fs::path filePath );

private:

	std::vector<SequencerRef> mSubSequences;

};

