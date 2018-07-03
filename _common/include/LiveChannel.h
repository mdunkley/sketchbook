#pragma once

#include <memory>
#include <stdio.h>
#include "cinder/app/App.h"
#include "cinder/audio/Buffer.h"
#include "cinder/audio/Context.h"
#include "Sequencer.hpp"
#include "SampleBank.h"
#include "Pattern.h"
#include "cinder/Json.h"

typedef std::shared_ptr<class LiveChannel> LiveChannelRef;
typedef std::shared_ptr<class LiveChannelBank> LiveChannelBankRef;
typedef std::shared_ptr<struct LiveSceneData> LiveSceneDataRef;

using ci::JsonTree;

struct LiveSceneData {

	bool active;

	ci::audio::BufferRef buffer;

	float length;
	float beats;
	int divisions;

	PatternRef slicePattern;
	std::vector<float> slices;

	PatternRef divPattern;
	std::vector<int> activeDivs;
};

class LiveChannel
{
public:

	LiveChannel();
	LiveChannel(fs::path jsonPath, int ticks=16);
	~LiveChannel();

	size_t size() { return mSceneLookup.size(); }
	bool isEmpty() { return size() == 0; }

	LiveSceneDataRef getScene(int sceneId);
	int				getSceneCount();

	void setName(string name) { mName = name; }
	string getName() const { return mName; }


	void loadFromJSON(fs::path path);

	std::string	mName;

	void setTicks(int ticks) { mTicks = ticks; }
	int getTicks() const { return mTicks; }


private:

	int mScene = 0;
	int mTicks = 16;
	int mSceneCount = 0;
	SampleBankRef	mSampleBank;
	PatternBankRef  mPatterns;
	std::map<int, LiveSceneDataRef>	mSceneLookup;
	LiveSceneDataRef mEmpty;
	//std::map<int, vector<float>> mSlices;
	
};

class LiveChannelBank {

public:

	LiveChannelBank();
	~LiveChannelBank();

	void addChannel(std::string name, PatternRef pat);

	void addChannel(PatternRef pat);

	size_t size() { return mChannels.size(); }
	bool isEmpty() { return size()== 0; }
	LiveChannelRef getChannel(std::string name);
	LiveChannelRef getRandomChannel();
	std::vector<LiveChannelRef> getChannels();
	std::vector<std::string> getChannelNames();

	void loadChannelsFromDirectory(fs::path path, int ticks = 0);

	void loadChannelsFromAssetsDirectory(std::string directory, int ticks=0);

	int getMaxScenes();

	size_t size() const { return mChannels.size(); }

	bool isEmpty() const { return mChannels.size() == 0; }
	void setTicks(int ticks) { mTicks = ticks; }
	int getTicks() const { return mTicks; }

private:
	int mTicks = 16;
	std::map<std::string, LiveChannelRef> mChannels;
	LiveChannelRef mEmptyChannel;

};
