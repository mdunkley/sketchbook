//
//  AudioManager.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/3/18.
//

#pragma once

#include <stdio.h>

#include "cinder/app/App.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/PanNode.h"
#include "SampleBank.h"
#include "SampleVoice.hpp"
#include "LiveChannel.h"

typedef std::shared_ptr<class Instrument>			InstrumentRef;
typedef std::shared_ptr<class WavetableVoice>		WavetableVoiceRef;
typedef std::shared_ptr<class PolyWavetablePlayer>	PolyWavetablePlayerRef;
typedef std::shared_ptr<class SampleVoice>			SampleVoiceRef;
typedef std::shared_ptr<class PolySamplePlayer>		PolySamplePlayerRef;
typedef std::shared_ptr<class SequencedSample>		SequencedSampleRef;
typedef std::shared_ptr<class GranularPlayer>		GranularPlayerRef;
typedef std::shared_ptr<class Recorder>				AudioRecorderRef;
typedef std::shared_ptr<class AudioManager>			AudioManagerRef;

class AudioManager {
    
    
  public:
    
    AudioManager();
    void update();
	void loadAssetAudio();
	void loadChannelBank( string name, string assetpath, int ticks=16 );



	SampleBankRef					getSampleBank(string name);
	LiveChannelBankRef				getChannelBank(string name);

	float							getRMS();
  
	ci::audio::NodeRef				getOutput() const { return mMasterPan; }
	void							setVolume(float v) { mMasterGain->setValue(v); }
	float							getVolume() const { return mMasterGain->getValue(); }

	WavetableVoiceRef				initializeWavetableVoice();
	WavetableVoiceRef				getWavetableVoice();
	vector<WavetableVoiceRef>		getWavetableVoices(int count);

    SampleVoiceRef					initializeSampleVoice();
	SampleVoiceRef					getSampleVoice();
	SampleVoiceRef					getSampleVoice(ci::audio::BufferRef src);
	list<SampleVoiceRef>			getSampleVoices(int count);
	list<SampleVoiceRef>            getSampleVoices(const std::vector< fs::path > paths);

	PolySamplePlayerRef				initializePolySamplePlayer();
	PolySamplePlayerRef				getPolySamplePlayer(bool exclusive = true);
	list<PolySamplePlayerRef>		getPolySamplePlayers(int count);

	SequencedSampleRef				initializeSequencedSample();
	SequencedSampleRef				getSequencedSample(bool exclusive = true );
	vector<SequencedSampleRef>		getSequencedSamples(int count);

    GranularPlayerRef				initializeGranularPlayer();
	GranularPlayerRef				getGranularPlayer();
	GranularPlayerRef				getGranularPlayer( ci::audio::BufferRef src );
	list<GranularPlayerRef> 		getGranularPlayers(int count);
	list<GranularPlayerRef>         getGranularPlayers( const std::vector< fs::path > paths );

    PolyWavetablePlayerRef			getPolyWavetablePlayer();
    AudioRecorderRef                getAudioRecorder();


	std::vector<PolySamplePlayerRef>&	getAllPolySamplePlayers() { return mPolySamplePlayers; }
	std::vector<GranularPlayerRef>&		getAllGranularPlayers() { return mGranularPlayers;  }

  private:

	std::vector<WavetableVoiceRef>	mWavetableVoices;
	std::vector<SampleVoiceRef>		mSampleVoices;
	std::vector<SequencedSampleRef>	mSequencedSamples;
	std::vector<GranularPlayerRef>	mGranularPlayers;
	std::vector<PolySamplePlayerRef>	mPolySamplePlayers;
	std::vector<InstrumentRef>		mAllInstruments;
	ci::audio::MonitorNodeRef		mMonitor;
    ci::audio::GainNodeRef			mMasterGain;
    ci::audio::Pan2dNodeRef			mMasterPan;
    audio::Context*					mCtx;

	std::map<std::string, SampleBankRef> mAllSampleBanks;
	std::map<std::string, LiveChannelBankRef> mAllChannelBanks;

	friend class Editor;
    
};
