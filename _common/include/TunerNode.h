#pragma once

#include <array>
#include <stdio.h>
#include "cinder/audio/audio.h"

using namespace cinder::audio;

typedef std::shared_ptr<class TunerNode>	TunerNodeRef;

class TunerNode : public ci::audio::Node
{

public:

	TunerNode(const Format &format = Format());
	~TunerNode();

	void calibrate();

protected:

	void	initialize()							override;
	void	process(ci::audio::Buffer *buffer)		override;



private:

	std::array<float,128> mLookup;
	double	mSampleRate = 48000;
	double	mAccum = 0.0;
	double	mFrequency = 0;
	int		mDetectedNote = 0;
	double	mPlayedValue = 0;
	double	mPrevFrequency = 0;
	size_t	mCount = 0;
	bool	mPrevSign = false;
	bool	mCalibrate = false;

};

