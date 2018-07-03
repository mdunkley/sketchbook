//
//  AudioManager.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/3/18.
//

#include "AudioManager.hpp"

#include "Instrument.hpp"
#include "WavetableVoice.hpp"
#include "GranularPlayer.hpp"
#include "PolyWavetablePlayer.hpp"
#include "PolySamplePlayer.hpp"
#include "SequencedSample.h"
#include "Recorder.hpp"

#include "cinder/app/App.h"
#include "cinder/audio/Context.h"
#include "cinder/Camera.h"
#include "cinder/audio/Utilities.h"
#include "cinder/Log.h"

using namespace ci;
using namespace std;

AudioManager::AudioManager()
{
    mCtx = audio::master();
    mMasterGain = mCtx->makeNode( new audio::GainNode( 1.0f ) );
    mMasterPan  = mCtx->makeNode( new audio::Pan2dNode() );
	mMonitor = mCtx->makeNode(new audio::MonitorNode());
    mMasterGain >> mMasterPan >> mCtx->getOutput();
	mMasterPan >> mMonitor;
    mMasterGain->enable();

    mCtx->enable();
}

void AudioManager::update(){
    
    for( auto it=mAllInstruments.begin(); it!=mAllInstruments.end(); it++) (*it)->update();
}

void AudioManager::loadAssetAudio() {
	mAllSampleBanks = SampleBank::loadAllFilesInAssets();
}

void AudioManager::loadChannelBank(string name, string assetpath, int ticks)
{
	LiveChannelBankRef bank = make_shared<LiveChannelBank>();
	bank->setTicks(ticks);
	bank->loadChannelsFromAssetsDirectory(assetpath);
	mAllChannelBanks[name] = bank;

}



LiveChannelBankRef AudioManager::getChannelBank(string name) {
	return mAllChannelBanks.at(name);
}


SampleBankRef AudioManager::getSampleBank(string name)
{
	return mAllSampleBanks[name];
}

float AudioManager::getRMS() {
	if (!mMonitor->isEnabled()) mMonitor->enable();
	return mMonitor->getVolume();
}

WavetableVoiceRef AudioManager::initializeWavetableVoice()
{
    WavetableVoiceRef s = std::make_shared<WavetableVoice>();
	size_t instId = mAllInstruments.size();
    s->setName("WavetableVoice_" + std::to_string(instId));
    audio::NodeRef out = s->getBusOutput();
    mAllInstruments.push_back( s );
	mWavetableVoices.push_back(s);
    out >> mMasterGain;
    return s;

}

WavetableVoiceRef AudioManager::getWavetableVoice()
{
	WavetableVoiceRef r;
	bool found = false;

	for (WavetableVoiceRef p : mWavetableVoices) {
		//ci::app::console() << "WHY AM I HERE" << std::endl;
		if (!p->inUse() && (!p->isEnabled())) {
			r = p;
			found = true;
			break;
		}
	}
	
	if (!found) {
		WavetableVoiceRef instrument = initializeWavetableVoice();
		ci::app::console() << "Loading Wavetable Player " << instrument->getName() << std::endl;
		r = instrument;
	}
	return r;
}

vector<WavetableVoiceRef> AudioManager::getWavetableVoices(int count)
{
	vector<WavetableVoiceRef> r;

	for (int i = 0; i < count; i++) {
		auto p = getWavetableVoice();
		p->setInUse(true);
		r.push_back(p);
	}

	for (auto p : r) {
		p->setInUse(false);
	}

	return r;
}


SampleVoiceRef AudioManager::initializeSampleVoice()
{
    SampleVoiceRef s = std::make_shared<SampleVoice>();
    size_t instId = mAllInstruments.size();
    s->setName("SampleVoice_" + std::to_string(instId));
    audio::NodeRef out = s->getBusOutput();
    mAllInstruments.push_back( s );
    out >> mMasterGain;
	mSampleVoices.push_back(s);
    return s;
}

SampleVoiceRef	AudioManager::getSampleVoice(){
	SampleVoiceRef r;
	for (SampleVoiceRef p : mSampleVoices) {
		if (!p->inUse()&&(!p->isEnabled())) {
			r = p;

			break;
		}
	}
	if ( !r ) r = initializeSampleVoice();

	return r;
}

SampleVoiceRef	AudioManager::getSampleVoice(ci::audio::BufferRef src)
{
	SampleVoiceRef s = getSampleVoice();
	s->setBuffer(src);
	mSampleVoices.push_back(s);
	return s;
}

GranularPlayerRef AudioManager::initializeGranularPlayer()
{
    GranularPlayerRef s = std::make_shared<GranularPlayer>();
	size_t instId = mAllInstruments.size();
    s->setName("GranularPlayer_" + std::to_string(instId));
    audio::NodeRef out = s->getBusOutput();
    mAllInstruments.push_back( s );
    out >> mMasterGain;
	mGranularPlayers.push_back(s);
    return s;

}

GranularPlayerRef AudioManager::getGranularPlayer(ci::audio::BufferRef src){
	GranularPlayerRef s = getGranularPlayer();
	s->setBuffer(src);
	mGranularPlayers.push_back(s);
	return s;
}

PolySamplePlayerRef AudioManager::initializePolySamplePlayer()
{
	PolySamplePlayerRef s = std::make_shared<PolySamplePlayer>();
	size_t instId = mAllInstruments.size();
    s->setName("PolySamplePlayer_" + std::to_string(instId));
    audio::NodeRef out = s->getBusOutput();
    mAllInstruments.push_back( s );
	mPolySamplePlayers.push_back(s);
    out >> mMasterGain;
    return s;
}

PolySamplePlayerRef AudioManager::getPolySamplePlayer( bool exclusive )
{
	PolySamplePlayerRef r;
	bool found = false;
	for (PolySamplePlayerRef p : mPolySamplePlayers) {
		if (!p->inUse() ) {
			r = p;
			found = true;
			break;
		}
	}
	if (!found) {
		PolySamplePlayerRef instrument = initializePolySamplePlayer();
		ci::app::console() << "Loading PolySamplePlayer " << instrument->getName() << std::endl;
		r = instrument;
	}

	r->setInUse( exclusive );

	return r;
}

list<PolySamplePlayerRef> AudioManager::getPolySamplePlayers(int count)
{
	list<PolySamplePlayerRef> r;
	int currentCount = 0;
	for (int i = 0; i < count; i++) {
		auto p = getPolySamplePlayer();
		r.push_back(p);
	}
	return r;
}

SequencedSampleRef AudioManager::initializeSequencedSample()
{
	SequencedSampleRef s = std::make_shared<SequencedSample>();
	size_t instId = mAllInstruments.size();
	s->setName("SequencedSample_" + std::to_string(instId));
	audio::NodeRef out = s->getBusOutput();
	mAllInstruments.push_back(s);
	mSequencedSamples.push_back(s);
	out >> mMasterGain;
	return s;
}

SequencedSampleRef AudioManager::getSequencedSample(bool exclusive)
{
	SequencedSampleRef r;
	bool found = false;
	for (SequencedSampleRef p : mSequencedSamples) {
		if (!p->inUse()) {
			r = p;
			found = true;
			break;
		}
	}
	if (!found) {
		SequencedSampleRef instrument = initializeSequencedSample();
		ci::app::console() << "Loading SequencedSample " << instrument->getName() << std::endl;
		r = instrument;
	}

	r->setInUse(exclusive);
	//ci::app::console() << "Checking out " << r->getName() << std::endl;

	return r;
}

vector<SequencedSampleRef> AudioManager::getSequencedSamples(int count)
{
	vector<SequencedSampleRef> r;
	int currentCount = 0;
	for (int i = 0; i < count; i++) {
		auto p = getSequencedSample(true);
		r.push_back(p);
	}
	for (auto& s : r) {
		s->setInUse(false);
	}

	return r;
}

PolyWavetablePlayerRef AudioManager::getPolyWavetablePlayer()
{
    PolyWavetablePlayerRef s = std::make_shared<PolyWavetablePlayer>();
	size_t instId = mAllInstruments.size();
    s->setName("PolyWavetablePlayer_" + std::to_string(instId));
    audio::NodeRef out = s->getBusOutput();
    mAllInstruments.push_back( s );
    out >> mMasterGain;
    return s;
}

AudioRecorderRef AudioManager::getAudioRecorder()
{
    AudioRecorderRef s = std::make_shared<Recorder>();
    //mAllInstruments.push_back( s );
    return s;
}

GranularPlayerRef AudioManager::getGranularPlayer() 
{
	GranularPlayerRef r;
	bool found = 0;
	for (GranularPlayerRef p : mGranularPlayers) {
		if (!p->inUse()) {
			r = p;
			found = 1;
			break;
		}
	}
	if (!found) {
		GranularPlayerRef instrument = initializeGranularPlayer();
		r = instrument;
	}
	return r;
}

list<GranularPlayerRef> AudioManager::getGranularPlayers(int voicecount) {
	list<GranularPlayerRef> g;
	for (int i=0; i < voicecount; i++) {
		GranularPlayerRef s = getGranularPlayer();
		g.push_back(s);
	}

	return g;
}

list<GranularPlayerRef> AudioManager::getGranularPlayers(const std::vector< fs::path > paths)
{
	list<GranularPlayerRef> newInstruments;
	for(auto p: paths) {
		GranularPlayerRef instrument = getGranularPlayer();
		instrument->loadBuffer(p.string());
	}

	return newInstruments;
}

list<SampleVoiceRef> AudioManager::getSampleVoices(int count)
{
	list<SampleVoiceRef> r;
	int currentCount = 0;
	for (SampleVoiceRef p : mSampleVoices) {
		if (!p->inUse()) {
			r.push_back(p);
			if (r.size() >= count) {
				break;
			}
		}
	}
	if (r.size() < count) {
		size_t remaining = count - r.size();
		for (int i = 0; i < count; i++) {
			SampleVoiceRef instrument = initializeSampleVoice();
			r.push_back(instrument);
		}
	}
	return r;
}


list<SampleVoiceRef> AudioManager::getSampleVoices(const std::vector< fs::path > paths)
{
	list<SampleVoiceRef> newInstruments;
	for (auto p : paths) {
		SampleVoiceRef instrument = getSampleVoice();
		newInstruments.push_back(instrument);
	}
	return newInstruments;
}






