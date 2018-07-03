//
//  Instrument.hpp
//  Instrument
//
//  Created by Michael Dunkley on 3/3/18.
//

#ifndef Instrument_hpp
#define Instrument_hpp

#include <stdio.h>

#include "cinder/audio/Context.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/audio.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/PanNode.h"

using namespace ci;
using namespace std;

class AudioManager;

class Instrument {

public:
    
    Instrument();
    virtual ~Instrument();
    
    virtual void setName( string n = "instrument" ) { mName = n; }
    virtual string getName() const { return mName; }
    
    virtual void disable();
    virtual void enable();
    bool isEnabled() const { return mEnabled; }
    void setEnabled( bool v );
    void toggleEnabled() { mEnabled = !mEnabled; }
    
    virtual void update();
    
    double  getVoiceAge();

	virtual void resetParameters();

	void	setFxAutoEnable(bool b) { mFxAutoEnable = b; }

	bool	inUse() { return mInUse;  }
	void	setInUse(bool a) { mInUse = a; }
    
    void    setADSR( float a, float d, float s, float r ) { mAttack = a; mDecay = d; mSustain = s; mRelease = r; }
    void    setAttack( float v ) { mAttack = v; }
    float   getAttack() const {  return mAttack; }
	void    setAttackJitter(float v) { mAttackJitter = v; }
	float   getAttackJitter() const { return mAttackJitter; }
	void    setHold(float v) { mHold = v; }
	float   getHold() const { return mHold; }
	void    setHoldJitter(float v) { mHoldJitter = v; }
	float   getHoldJitter() const { return mHoldJitter; }
    void    setDecay( float v ) { mDecay = v; }
    float   getDecay() const { return mDecay; }
	void    setDecayJitter(float v) { mDecayJitter = v; }
	float   getDecayJitter() const { return mDecayJitter; }
    void    setSustain( float v ) { mSustain = v; }
    float   getSustain() const { return mSustain; }
	void    setSustainJitter(float v) { mSustainJitter = v; }
	float   getSustainJitter() const { return mSustainJitter; }
    void    setRelease( float v ) { mRelease = v; }
    float   getRelease() const {  return mRelease; }
	void    setReleaseJitter(float v) { mReleaseJitter = v; }
	float   getReleaseJitter() const { return mReleaseJitter; }


    
    void    setMonitorActive( bool v );
    bool    getMonitorActive() const { return mMonitorActive; };

    virtual float   getVolume() const { return mVolume; }
	virtual void    setVolume(float v) { mVolume = v; }
    virtual float   getPan() const { return mPan; }
    virtual void    setPan( float v );

    float   getRms();
	void	setRmsMult(float v) { mRmsMult = v; }
	float	getRmsMult() const { return mRmsMult; }
	void	setRmsSlewAttack(float attack) { mRmsSlewAttack = ci::clamp(attack, 0.0f, 1.0f); }
	void	setRmsSlewDecay(float decay) { mRmsSlewDecay = ci::clamp(decay, 0.0f, 1.0f); }
    
    virtual void setBusInput( audio::NodeRef n ) {mBusInputRef = n; }
    virtual audio::NodeRef getBusInput() const { return mBusInputRef; }
    virtual void setBusOutput( audio::NodeRef n ) {mBusOutputRef = n; }
    virtual audio::NodeRef getBusOutput() const { return mBusOutputRef; }

    virtual void	gate( bool g );
	virtual void	trigger( );
    virtual void	choke( float v = .002f );

    bool    mMonitorActive      = false;
    bool    mEnabled            = false;
    string  mName               = "";
    float   mVolume             = 1.0f;
    float   mPan                = 0.5f;
    float   mAttack             = 0.002f;
	float	mAttackJitter		= 0.0f;
	float	mHold				= 0.0f;
	float	mHoldJitter			= 0.0f;
    float   mDecay              = 0.002f;
	float	mDecayJitter		= 0.0f;
    float   mSustain            = 1.0f;
	float	mSustainJitter		= 0.0f;
    float   mRelease            = .002f;
	float	mReleaseJitter		= 0.0f;
    bool    mGate               = false;
	bool	mInUse				= false;
    double  mActivationTime		= 0.0;
	float	mRmsSlewAttack		= .96f;
	float	mRmsSlewDecay		= .05f;
	float	mRmsSlew = 0.0;
	float	mRmsMult = 10.0f;
	bool	mFxAutoEnable = false;
    
    audio::NodeRef          mBusInputRef;
    audio::NodeRef          mBusOutputRef;
    list<audio::NodeRef>    mNodes;

    audio::GainNodeRef      mGainRef;
    audio::Pan2dNodeRef     mPanRef;
    audio::MonitorNodeRef   mMonitorRef;
    
    audio::EventRef         mAttackRamp;
	audio::EventRef			mHoldRamp;
    audio::EventRef         mDecayRamp;
    audio::EventRef         mReleaseRamp;
    
    AudioManager*           mParent;

	std::vector<ci::audio::NodeRef> mFxNodes;

};

#endif /* Instrument_hpp */

