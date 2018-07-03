	#pragma once

#include <stdio.h>
#include <memory>
#include "cinder/app/App.h"
#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/Buffer.h"
#include "cinder/audio/Context.h"
#include "cinder/Rand.h"
#include "Pattern.h"



using namespace ci;
using namespace std;

typedef std::shared_ptr<class SampleBank> SampleBankRef;
typedef std::shared_ptr<struct BufferInfo> BufferInfoRef;

struct BufferInfo{

	ci::audio::BufferRef buffer;
	vector<double> slices;
	std::map<int, int> noteMap;
	std::vector<PatternRef> patterns;

};

class SampleBank
{
public:

	SampleBank();
	~SampleBank();

	void setName(std::string name) { mName = name; }
	std::string& getName() { return mName; }

	void addBuffers( std::vector< fs::path > paths);
	void addBuffers( std::vector < BufferInfoRef>& buffers);
	ci::audio::BufferRef addBuffer(BufferInfoRef r);
	ci::audio::BufferRef addBuffer(fs::path bufferPath);
	void loadDirectory(fs::path path);
	void loadDirectory(const string path);
	void loadAssetDirectory(const string path);
	void loadAssetDirectoryByName(const string path);
	BufferInfoRef loadColl(fs::path file);
	size_t getSize() { return mBuffers.size(); }
	std::vector<ci::audio::BufferRef>& getBuffers();
	std::vector<BufferInfoRef> getBuffersWithInfo();
	BufferInfoRef getNamedBufferInfo(std::string name);
	void printBufferNames();
	ci::audio::BufferRef getBuffer(size_t id) { return mBuffers.at(std::min(id, getSize())); }
	ci::audio::BufferRef getBufferByLookup(double l);
	ci::audio::BufferRef getRandomBuffer();
	ci::audio::BufferRef getNextBuffer();
	BufferInfoRef getRandomBufferInfo();

	bool hasBufferInfo(ci::audio::BufferRef buffer);
	BufferInfoRef getBufferInfo(ci::audio::BufferRef buffer);

	static std::map<std::string,SampleBankRef> loadAllFilesInAssets();


private:


	static std::vector<fs::path> getAllAssetDirectories();
	
	fs::path getCollPath(fs::path audioFile);
	void filesInDirectory(const fs::path& root, const string& ext, vector<fs::path>& ret);
	
	static std::string& stripAssetPath(std::string& sourceString);
	static std::string stripToName(fs::path sourcePath);
	
	std::vector<ci::audio::BufferRef> mBuffers;
	std::map<ci::audio::BufferRef, BufferInfoRef> mBufferInfo;
	std::map<std::string, BufferInfoRef > mNamedBufferInfo;
	std::string mName;

	int mCurrentBuffer = 0;

};

