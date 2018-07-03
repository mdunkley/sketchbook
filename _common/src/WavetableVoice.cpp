//
//  WavetableVoice.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/15/18.
//

#include "WavetableVoice.hpp"

WavetableVoice::WavetableVoice(){
	mOsc = audio::master()->makeNode(new audio::GenOscNode());
	mReleaseRamp = mGainRef->getParam()->applyRamp(0, INT_MAX);
	mGainRef->setValue(0);
	setFreqMidi(64);
	mOsc >> getBusInput();
	mNodes.push_back(mOsc);
	disable();
}

void WavetableVoice::setWaveform( string w ) {
    if( w=="sine" )             mOsc->setWaveform( audio::WaveformType::SINE );
    else if( w=="square" )      mOsc->setWaveform( audio::WaveformType::SQUARE );
    else if( w=="sawtooth" )    mOsc->setWaveform( audio::WaveformType::SAWTOOTH );
    else if( w=="triangle" )    mOsc->setWaveform( audio::WaveformType::TRIANGLE );
}

void WavetableVoice::setWaveform(audio::WaveformType  t) {
	mOsc->setWaveform(t);
}

float WavetableVoice::getFreq() const {
    return  mFreq ;
}

void WavetableVoice::setFreqMidi( float freqMidi ) {
    mFreq = ci::audio::midiToFreq(quantizeMidiNote( freqMidi) );
    mOsc->setFreq( mFreq );
}

void WavetableVoice::setFreq( float freqHz ) {
    mFreq = freqHz;
    mOsc->setFreq( mFreq );
}

void WavetableVoice::setScale(std::vector<int> scale) {

	mScale = scale;
	vector<int> fullScale;
	fullScale.push_back(mScale.back() - 12);
	fullScale.insert(fullScale.end(), mScale.begin(), mScale.end());
	fullScale.push_back(mScale[0] + 12);
	mSpreadScale = fullScale;
}

int WavetableVoice::quantizeMidiNote(int n) {

	if (mScale.size() > 0) {

		int qn = floor(n);
		int interval = qn % 12;
		int octave = qn / 12;
		int minDist = 9999;
		int minVal = 9999;
		for (int i = 0; i < mSpreadScale.size(); i++) {
			int dist = int(abs(interval - mSpreadScale.at(i)));
			if (dist < minDist) {
				minVal = mSpreadScale.at(i);
				minDist = dist;
			}
		}
		interval = minVal;
		return interval + (octave * 12);
	}
	else return n;
	
}