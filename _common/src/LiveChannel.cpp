#include "LiveChannel.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace ci::log;

LiveChannel::LiveChannel()
{
	mSampleBank = make_shared<SampleBank>();
	mEmpty = make_shared<LiveSceneData>();

	mEmpty->active = false;
}

LiveChannel::LiveChannel(fs::path jsonPath, int ticks) {

	mEmpty = make_shared<LiveSceneData>();
	mEmpty->active = false;
	mTicks = ticks;
	mSampleBank = make_shared<SampleBank>();

	loadFromJSON(jsonPath);
}


LiveChannel::~LiveChannel()
{
}

LiveSceneDataRef LiveChannel::getScene(int sceneId)
{
	if (!mSceneLookup.empty()) {
		std::map<int, LiveSceneDataRef>::iterator it;
		it = mSceneLookup.find(sceneId);
		if (it != mSceneLookup.end()) {
			return it->second;
		}
		else return mEmpty;
	}
	else return mEmpty;
}

int LiveChannel::getSceneCount()
{
	return mSceneCount;
}

void LiveChannel::loadFromJSON(fs::path path) {

	fs::path currentDirectory = path.parent_path();
	std::string jsonString = loadString(ci::loadFile(path.string()));

	JsonTree tree = JsonTree(jsonString);
	auto top = tree.getChildren();



	for (auto& c : top) {

		string channelName = c.getKey();
		int numScenes = 0;

		for (auto& scene : c.getChildren()) {

			int sceneId = stoi(scene.getKey());
			float volume = 0;
			int divisions = 0;
			float length = 0;
			float beats = 0;
			string file = "";
			std::vector<int> activeDivs;
			std::vector<float> slices;
			
			for (JsonTree::ConstIter cIt = scene.begin(); cIt != scene.end(); cIt++) {

				string key = cIt->getKey();

				if (key == "file") {
					file = cIt->getValue();
				}
				else if (key == "volume") {
					volume = cIt->getValue<float>();
				}
				else if (key == "divisions") {
					divisions = cIt->getValue<int>();
				}
				else if (key == "length") {
					length = cIt->getValue<float>();
				}
				else if (key == "beats") {
					beats = cIt->getValue<float>();
				}
				else if (key == "slices") {
					for (auto& s : cIt->getChildren())
						slices.push_back(s.getValue<float>());
				}
				else if (key == "activedivs") {
					for (auto& s : cIt->getChildren())
						activeDivs.push_back(s.getValue<float>());
				}

			}

			setName(channelName);
			LiveSceneDataRef data = make_shared<LiveSceneData>();

			fs::path audioPath = currentDirectory / file;
			if (fs::exists(audioPath) && fs::is_regular_file(audioPath) && !fs::is_directory(audioPath)) {
				ci::app::console() << "Loading " << audioPath << std::endl;

				data->active = true;
				data->divPattern = Pattern::patternFromDivisions(divisions, length, beats, mTicks );
				data->slicePattern = Pattern::patternFromSlices(slices, length, beats, mTicks);
				data->slices = slices;
				data->activeDivs = activeDivs;
				data->buffer = mSampleBank->addBuffer(audioPath);
				data->divisions = divisions;
				data->length = length;
			}
			else {
				data->active = false;
			}

			mSceneLookup[sceneId] = data;
			numScenes++;

		}

		mSceneCount = numScenes;

	}
}

LiveChannelBank::LiveChannelBank()
{
}

LiveChannelBank::~LiveChannelBank()
{
}

void LiveChannelBank::addChannel(std::string name, PatternRef pat)
{
}

void LiveChannelBank::addChannel(PatternRef pat)
{
}

LiveChannelRef LiveChannelBank::getChannel(std::string name)
{
	return mChannels.at(name);


}

LiveChannelRef LiveChannelBank::getRandomChannel() {

	if (!isEmpty()) {

		int lookup = Rand::randInt(size()-1);
		std::map<std::string,LiveChannelRef>::iterator it = mChannels.begin();
		std::advance(it, lookup);

		return (*it).second;
	}
	else {
		return mEmptyChannel;
	}
}

std::vector<LiveChannelRef> LiveChannelBank::getChannels()
{

	std::vector<LiveChannelRef> allChannels;
	for (auto& c : mChannels) {
		allChannels.push_back(c.second);
	}
	return allChannels;
}

std::vector<std::string> LiveChannelBank::getChannelNames()
{
	std::vector<std::string> names;
	if( size() > 0 ){ 
		for (auto& c : mChannels) {
			names.push_back(c.first);
		}
	}
	return names;
}

void LiveChannelBank::loadChannelsFromDirectory(fs::path path, int ticks) {

	if (ticks > 0) {
		mTicks = ticks;
	}

	for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it)
	{
		if (fs::is_regular_file(*it))
		{
			std::string ext = it->path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (fs::is_regular_file(*it) &&	(ext == ".json" )) {
				string jsonString = loadString(ci::loadFile(fs::path(*it).string()));
				LiveChannelRef channel = make_shared<LiveChannel>(fs::path(*it).string(), mTicks);
				mChannels[channel->getName()] = channel;
			}
		}
	}
}

void LiveChannelBank::loadChannelsFromAssetsDirectory(std::string directory, int ticks)
{
	if (ticks > 0) mTicks = ticks;
	
	for (auto& basedir : ci::app::getAssetDirectories()) {
		fs::path fullpath = basedir / directory;
		if (fs::exists(fullpath)) {
			//ci::app::console() << mTicks << std::endl;
			loadChannelsFromDirectory(fullpath, mTicks);
			break;
		}
	}
}

int LiveChannelBank::getMaxScenes() {
	int s = 0;
	for (auto& c : mChannels) {
		s = std::max(s, c.second->getSceneCount() );
	}
	return s;
}
