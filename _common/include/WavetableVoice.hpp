//
//  WavetableVoice.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/15/18.
//

#ifndef WavetableVoice_hpp
#define WavetableVoice_hpp

#include <stdio.h>
#include "Instrument.hpp"
#include "cinder/audio/audio.h"
#include "cinder/audio/GenNode.h"

using namespace ci;
using namespace std;

typedef std::shared_ptr<class WavetableVoice>	WavetableVoiceRef;

class WavetableVoice : public Instrument {

public:

	WavetableVoice();

	void		setWaveform( audio::WaveformType  t );
    void        setWaveform( string w ); // sine, square, sawtooth, triangle
    float       getFreq() const;
    void        setFreqMidi( float freqMidi );
    void        setFreq( float freqHz );

	void				setScale( std::vector<int> );
	std::vector<int>	getScale() const { return mScale; }

private:

	int quantizeMidiNote(int n);
	std::vector<int> mScale;
	std::vector<int> mSpreadScale;

    audio::GenOscNodeRef  mOsc;
    float mFreq = 200.0f;

};




#endif /* WavetableVoice_hpp */
