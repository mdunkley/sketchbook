
#include "SampleBank.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include <fstream>
#include <stdio.h>


SampleBank::SampleBank()
{
}

SampleBank::~SampleBank()
{
}

std::vector<ci::audio::BufferRef>& SampleBank::getBuffers() {
	return mBuffers;
}

ci::audio::BufferRef SampleBank::getBufferByLookup(double l) {
	int id = round(clamp(l,0.0,1.0) *(getSize() - 1));
	return getBuffer(id);
}

ci::audio::BufferRef SampleBank::addBuffer(fs::path bufferPath) {

	if (fs::exists(bufferPath)) {

		audio::SourceFileRef fileRef = audio::load(loadFile(bufferPath), ci::audio::master()->getSampleRate());
		ci::audio::BufferRef buf = fileRef->loadBuffer();
		mBuffers.push_back(buf);

		fs::path coll = getCollPath(bufferPath);

		if (coll != ".") {
			BufferInfoRef ref = loadColl(coll);
			ref->buffer = buf;
			mBufferInfo.insert(std::pair<ci::audio::BufferRef, BufferInfoRef>(buf, ref));
			string name = bufferPath.filename().string();
			mNamedBufferInfo[name] = ref;
		}
		return buf;
	}
	else return nullptr;
}

ci::audio::BufferRef SampleBank::addBuffer(BufferInfoRef r) {
	mBuffers.push_back(r->buffer);
	mBufferInfo.insert(std::pair<ci::audio::BufferRef, BufferInfoRef>(r->buffer, r));
	return r->buffer;
}

void SampleBank::addBuffers( std::vector <BufferInfoRef>& buffers) {
	for (auto b : buffers) {
		addBuffer(b);
	}
}

void SampleBank::addBuffers( std::vector< fs::path > paths)
{
	for (auto p : paths) {
		addBuffer(p);
	}
}

void SampleBank::loadDirectory(fs::path path) {

	for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it)
	{
		if (fs::is_regular_file(*it))
		{
			std::string ext = it->path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (fs::is_regular_file(*it) &&
				(ext == ".wav" || ext == ".mp3")) {
				addBuffer(*it);
				ci::app::console() << "Loading File " << it->path().string() << std::endl;
			}

		}
	}
}

void SampleBank::loadDirectory(const string path) {

	fs::path dir(path);
	loadDirectory(dir);

}

void SampleBank::printBufferNames() {
	for (auto& name : mNamedBufferInfo) {
		ci::app::console() << name.first << std::endl;
	}
}

void SampleBank::loadAssetDirectory(const string path) {

	for (auto& dir : ci::app::getAssetDirectories()) {
		fs::path testPath = dir / path;
		if (fs::exists(testPath)) {
			
			loadDirectory(testPath);
			break;
		}
	}

}

void SampleBank::loadAssetDirectoryByName(const string path) {

	for ( auto& dir : ci::app::getAssetDirectories() ) {

		fs::recursive_directory_iterator it(dir);
		fs::recursive_directory_iterator endit;

		while (it != endit) {

			if (fs::exists(*it) && fs::is_directory(*it)) {
				if (fs::path(*it).filename() == path) {
					loadDirectory(*it);
					break;
				}
			}
			++it;
		}
	}
}

void SampleBank::filesInDirectory(const fs::path& root, const string& ext, vector<fs::path>& ret)
{
    if(!fs::exists(root) || !fs::is_directory(root)) return;

    fs::recursive_directory_iterator it(root);
    fs::recursive_directory_iterator endit;

    while(it != endit)
    {
        if(fs::is_regular_file(*it) && it->path().extension() == ext) ret.push_back(it->path().filename());
        ++it;
    }

}

std::map<std::string, SampleBankRef> SampleBank::loadAllFilesInAssets() {
	
	std::map < std::string, SampleBankRef > ret;

	for (auto& dir : getAllAssetDirectories()) {

		ci::app::console() << "Found " << dir << std::endl;
		fs::directory_iterator it(dir);
		fs::directory_iterator endit;

		std::vector<fs::path> files;

		while (it != endit) {

			std::string ext = it->path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if	(fs::is_regular_file(*it) &&
				(ext == ".wav" || ext == ".mp3")) {

				files.push_back(*it);
			}
			++it;
		}

		if (files.size() > 0) {
			std::vector<std::string> dirs;
			std::string name = stripAssetPath( (dir/".").parent_path().string() );
			SampleBankRef bank = make_shared<SampleBank>();
			bank->addBuffers(files);
			ret.insert(std::pair<std::string, SampleBankRef>(name, bank));
		}
	}
	for (auto& m : ret) {
		ci::app::console() << m.first << " " << m.second << std::endl;
	}
	return ret;
}

std::vector<fs::path> SampleBank::getAllAssetDirectories() {

	std::vector<fs::path> allPaths;

	for (auto& assetdir : ci::app::getAssetDirectories()) {

		allPaths.push_back(assetdir);

		fs::recursive_directory_iterator it(assetdir);
		fs::recursive_directory_iterator endit;

		while (it != endit) {
			if (fs::is_directory(*it)) {
				allPaths.push_back(*it);
			}
			++it;
		}
	}

	return allPaths;
}


BufferInfoRef SampleBank::getNamedBufferInfo(std::string name) {
	return mNamedBufferInfo.at(name);

}

std::vector<BufferInfoRef> SampleBank::getBuffersWithInfo() {

	std::vector<BufferInfoRef> infoBuffers;
	for (auto& b : mBufferInfo) {
		infoBuffers.push_back(b.second);
	}
	return infoBuffers;

}

std::string& SampleBank::stripAssetPath(std::string& sourceString) {

	// Remove the asset directory from path to get unique directory keys
	for (auto& path : app::getAssetDirectories()) {
		std::string::size_type pos = sourceString.find(path.string());
		if (pos != std::string::npos) {
			sourceString.erase(pos, path.parent_path().string().size()+1);
			break;
		}
	}
	return sourceString;
}

std::string SampleBank::stripToName(fs::path sourcePath) {

	std::string pathString = sourcePath.filename().string();
	size_t lastIndex = pathString.find_last_of(".");
	return pathString.substr(0, lastIndex);

	return pathString;
}

BufferInfoRef SampleBank::loadColl(fs::path file) {

	BufferInfoRef info( new BufferInfo );
	std::set<int> allNotes;

	std::string line;
	std::ifstream myFile( file.string() );

	int numPatterns = 0;

	if (myFile.is_open())
	{
		allNotes.clear();
		while (getline(myFile, line))
		{

			std::vector<std::string> countAndContent = ci::split(line, ",", false);
			string id = countAndContent.at(0);
			std::vector<std::string> content(ci::split(countAndContent.at(1), " ", false));
			content.erase(content.begin());


			if (id == "slices") {
				for( int i=0; i<content.size()/2; i++) {
					int id = stoi(content.at(i * 2));
					float position = stof(content.at(i * 2 + 1));
					info->slices.push_back(position);
					info->noteMap.insert(std::pair<int, int>(id, i));

				}
			}
			else {

				PatternRef pattern = make_shared<Pattern>();  // New pattern per line
				int numNotes = content.size() / 4;


				int length = stoi(content.at(0));

				for (int i = 0; i < numNotes; i++) {

					int offset = (i * 4) + 1;  // first value is empty

					int step = std::floor(stof(content.at(offset + 1)) * 16);
					int pitch = std::floor(stof(content.at(offset)));
					float duration = stof(content.at(offset + 2));
					float velocity = stof(content.at(offset + 3));
					NoteEvent note = { pitch, duration, velocity };

					pattern->addNote(step, note);
					allNotes.insert(pitch);
				}
				pattern->setLength(length * 16);
				info->patterns.push_back(pattern);
				numPatterns++;
			}

		}

		myFile.close();
	}
	else ci::app::console() << "Unable to open file" << std::endl;

	return info;

}

fs::path SampleBank::getCollPath(fs::path audioFile) {

	std::string fileName = stripToName(audioFile) + ".coll";

	fs::path collFile = audioFile.parent_path() / fileName;
	if (fs::exists(collFile)) {
		
		return fs::canonical(collFile);
	}
	else {
		return ".";
	}
}

ci::audio::BufferRef SampleBank::getRandomBuffer() {
	if (getSize() > 0) {
		mCurrentBuffer = Rand::randInt(getSize());
		return mBuffers.at(mCurrentBuffer);

	} else return nullptr;
}

ci::audio::BufferRef SampleBank::getNextBuffer() {
	int size = getSize();
	if (size > 0) {
		mCurrentBuffer++;
		if (mCurrentBuffer >= size) mCurrentBuffer = 0;
		return mBuffers.at(mCurrentBuffer);
	}
	else return nullptr;
}

BufferInfoRef SampleBank::getRandomBufferInfo() {

	auto buffers = getBuffersWithInfo();
	float s = buffers.size();
	if (s > 0) return buffers.at(Rand::randInt(s - 1));
		else return nullptr;

}

bool SampleBank::hasBufferInfo(ci::audio::BufferRef buffer) {
	bool found = false;
	std::map<ci::audio::BufferRef, BufferInfoRef>::iterator val = mBufferInfo.find(buffer);
	if (val != mBufferInfo.end()) found = true;
	return found;
}

BufferInfoRef SampleBank::getBufferInfo(ci::audio::BufferRef buffer) {
	std::map<ci::audio::BufferRef, BufferInfoRef>::iterator val = mBufferInfo.find(buffer);
	if (val != mBufferInfo.end()) return val->second;
	else return nullptr;
}
