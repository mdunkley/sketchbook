#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"

#include "Recorder.hpp"
#include "AudioManager.hpp"
#include "ParticleSystem.hpp"
#include "WavetableVoice.hpp"
#include "PolySamplePlayer.hpp"
#include "Sequencer.hpp"
#include "PolyWavetablePlayer.hpp"
#include "GranularPlayer.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;


class AudioParticle : public Particle {
public:
    InstrumentRef instrument;
};

class AudioSandboxApp : public App {

  public:


	void setup() override;
    void resize() override;
	void mouseDown( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void fileDrop( FileDropEvent event ) override;
	void update() override;
	void draw() override;
    
    void setMouseModulators( MouseEvent event );
    
    void triggerGranularParticles();
    void triggerNoteParticles();
    
    bool gate = false;
    
    void loadSynthVoice();
    void loadPolySynthVoice();
    void loadSampleVoice();
    void loadGranularPlayer();
    void loadPolySampleVoice();
    void checkActiveInstruments();
    int quantizeMidiNote( float v, vector<int> s );
    int mOldNote;

	void triggerGranularPlayer();

    float   mMouseSpeed;
    vec2    mMousePos;
    vec2    mMouseVel;
    vec2    mRelMousePos;
    vec2    mPrevPos;
    vec2    mWindowSize;
    vec3    mCenterDist;
    
	AudioManagerRef					mAudioManager;
    WavetableVoiceRef				mSynthVoice;
    SampleVoiceRef					mSampleVoice;
    PolySamplePlayerRef				mPolySamplePlayer;
    GranularPlayerRef				mGranularPlayer;
    PolyWavetablePlayerRef			mPolySynthVoice;
    AudioRecorderRef				mRecorder;
    
    std::shared_ptr<ParticleSystem>         mParticleSystem;
    std::shared_ptr<ParticleSystem>         mNoteParticles;
    
    vector<int>             mScale;
    
    bool doSynthVoice= true;
    bool doPolySynthVoice= true;
    bool doSampleVoice=     true;
    bool doGranularPlayer= true;
    bool doPolySamplePlayer= true;
    
	Sequencer mClock;
    Sequencer mDiv;

};



void AudioSandboxApp::setup()
{
    mParticleSystem =   std::make_shared<ParticleSystem>();
    mNoteParticles  =	std::make_shared<ParticleSystem>();
    
    mAudioManager = std::make_shared<AudioManager>();
    if( mAudioManager ){

        if( doSynthVoice )          loadSynthVoice();
        if( doPolySynthVoice )      loadPolySynthVoice();
        if( doSampleVoice )         loadSampleVoice();
        if( doGranularPlayer )      loadGranularPlayer();
        if( doPolySamplePlayer )    loadPolySampleVoice();
        
        mRecorder = mAudioManager->getAudioRecorder();
        mRecorder->attachTo( mAudioManager->getOutput() );
    }

    mClock.setRate( .5 );
    //mClock.getClockSignal().connect( [&](){mDiv.clock();} );
	mDiv.isClocked = true;


    mScale = {0,3,5,7,10};
    mWindowSize = app::getWindowSize();

}

void AudioSandboxApp::triggerGranularPlayer() {
	mGranularPlayer->trigger();
}

void AudioSandboxApp::resize(){
    
    mWindowSize = app::getWindowSize();
    
}

void AudioSandboxApp::loadSynthVoice(){
    console()<<"Loading Synth Voice\n";
    mSynthVoice = mAudioManager->getWavetableVoice();
    mSynthVoice->setVolume(.5f );
    mSynthVoice->setADSR(.1f, .2f, .1f, 1.0f);
    mSynthVoice->setMonitorActive( true );
    console()<<"Finished loading Synth Voice\n";
}

void AudioSandboxApp::loadPolySynthVoice(){
    console()<<"Loading PolySynth Voice\n";
    mPolySynthVoice = mAudioManager->getPolyWavetablePlayer();
    mPolySynthVoice->setWaveform("sine");
    mPolySynthVoice->setMonitorActive( true );
    mPolySynthVoice->setVolume(.4f );
    mPolySynthVoice->setADSR(.1f, .2f, .5f, 1.0f);
    console()<<"Finished loading PolySynth Voice\n";
}

void AudioSandboxApp::loadSampleVoice( ){
	console() << "Loading Sample Voice" << std::endl;
    audio::SourceFileRef fileRef = audio::load( loadAsset( "audio_02.mp3" ) );
    mSampleVoice = mAudioManager->getSampleVoice();
    mSampleVoice->setBuffer( fileRef->loadBuffer() );
    mSampleVoice->setMonitorActive( true );
    mSampleVoice->setADSR(0.1f,0.2f,0.1f, 1.0f);
	mSampleVoice->setLoopEnabled(true);

    console()<<"Finished loading Sample Voice" << std::endl;
}

void AudioSandboxApp::loadGranularPlayer(){
    console()<<"Loading Granular Voice\n";
    audio::SourceFileRef fileRef = audio::load( loadAsset( "audio_02.mp3" ) );
    mGranularPlayer = mAudioManager->getGranularPlayer();
    mGranularPlayer->setBuffer( fileRef->loadBuffer() );
    mGranularPlayer->setDuration( .1f );
    //mGranularPlayer->setADSR(0.1f,0.2f,0.1f,1.0f);
    mGranularPlayer->setMonitorActive( true );
    console()<<"Finished loading Granular Player\n";
}

void AudioSandboxApp::loadPolySampleVoice(){
	mPolySamplePlayer = mAudioManager->getPolySamplePlayer();
    console()<<"Loading Poly Sample Player" << std::endl;;
	audio::SourceFileRef fileRef = audio::load(loadAsset("audio_02.mp3"));
	mPolySamplePlayer->setBuffer(fileRef->loadBuffer());
	mPolySamplePlayer->setSliceDivisions(16);
    console()<<"Finished loading Poly Sample Player" << std::endl;;
}

void AudioSandboxApp::setMouseModulators(MouseEvent event ){
    
    mMousePos = event.getPos();
    mRelMousePos = vec2( float(event.getX())/mWindowSize.x, float(event.getY())/mWindowSize.y );
    float cdist = length(vec2(mRelMousePos-vec2(.5,.5)));
    float ydist = length(vec2(mRelMousePos-vec2(mRelMousePos.x,.5)));
    float xdist = length(vec2(mRelMousePos-vec2(.5,mRelMousePos.y)));
    mCenterDist = vec3(cdist,ydist,xdist);
    mMouseVel= mMousePos - mPrevPos;
    mMouseSpeed = length(mMousePos-mPrevPos);
    mPrevPos = mMousePos;
    
}

void AudioSandboxApp::keyDown( KeyEvent event ){

    char c = event.getChar();
    
    if ( c=='z' ) {
         doSynthVoice = !doSynthVoice;
         if( doSynthVoice && !mSynthVoice ) loadSynthVoice();
    }
    else if ( c=='x' ){
        doPolySynthVoice = !doPolySynthVoice;
        if( doPolySynthVoice && !mPolySynthVoice ) loadPolySynthVoice();
    }
    else if ( c=='c' ){
        doSampleVoice = !doSampleVoice;
        if( doSampleVoice && !mSampleVoice ) loadSampleVoice();
    }
    else if ( c=='v' ){
        doGranularPlayer = !doGranularPlayer;
        if( doGranularPlayer && !mGranularPlayer ) loadGranularPlayer();
    }
    else if ( c=='b' ){
        doPolySamplePlayer = !doPolySamplePlayer;
        if( doPolySamplePlayer && !mPolySamplePlayer ) loadPolySampleVoice();
    }
    else if ( c=='r' ){
        
        bool rec = !mRecorder->isRecording();
        mRecorder->record( !mRecorder->isRecording() );

        if( !rec ) {
            audio::BufferRef buf = mRecorder->getRecordedCopy();
            mSampleVoice->setBuffer( buf );
            mGranularPlayer->setBuffer( buf );
        }
    } else if ( c=='t' ){
        
        mDiv.active( !mDiv.isActive() );
        //if ( div.isActive() ) div.setDivision( 1.0f/Rand::randInt(1,8) );
        
    } else if ( c=='g' ) ci::app::console()<<audio::master()->printGraphToString();
}

void AudioSandboxApp::checkActiveInstruments() {

    if( doSynthVoice && !mSynthVoice ) loadSynthVoice();
    if( !mSampleVoice && doSampleVoice ) loadSampleVoice();
    if( !mPolySamplePlayer && doPolySamplePlayer ) loadPolySampleVoice();
    if( doPolySynthVoice && !mPolySynthVoice ) loadPolySynthVoice();
    if( doGranularPlayer && !mGranularPlayer ) loadGranularPlayer();

}

void AudioSandboxApp::mouseDown( MouseEvent event )
{
    setMouseModulators( event );
    checkActiveInstruments();
    //if ( div.isActive() ) div.setDivision( 1.0f/Rand::randInt(1,8) );
    gate = true;

    if( doSynthVoice ) {
        
        mSynthVoice->setAttack( .1f );
        mSynthVoice->setDecay( 1.0f );
        int note = quantizeMidiNote( mRelMousePos.x * 127, mScale );
        mOldNote = note;
        mSynthVoice->setFreqMidi( note );
        mSynthVoice->trigger();

    }
	
    if( doSampleVoice ) {
        
        mSampleVoice->setAttack( .1f );
        mSampleVoice->setRelease( .2f );
		mSampleVoice->setSampleStart(.1);
		mSampleVoice->setLoopEnabled(true);
		mSampleVoice->setLoopPosition(.25);
		mSampleVoice->setSampleEnd(.1035);
        //mSampleVoice->setPosition( mRelMousePos.x );
        //mSampleVoice->trigger();
        mSampleVoice->gate(true);
    }

    if( doPolySamplePlayer && mPolySamplePlayer ){
		int slice = floor(mRelMousePos.x * 8);
		//ci::app::console() << "Playing polysample slice " << slice << std::endl;
		mPolySamplePlayer->setSlice(slice);
		mPolySamplePlayer->setAttack(.003);
		mPolySamplePlayer->setHold(1.0);
		mPolySamplePlayer->setRelease(0.003);
		mPolySamplePlayer->trigger();
    }

    if( doPolySynthVoice ){
        int note = quantizeMidiNote( mRelMousePos.x * 127, mScale );
        mOldNote = note;
        mPolySynthVoice->setFreqMidi( note );
        triggerNoteParticles();
        mPolySynthVoice->trigger();
    }

    if( doGranularPlayer ) {
        mGranularPlayer->setPosition( mRelMousePos.x );
        //mGranularPlayer->setPositionJitter( .001 );
		mGranularPlayer->setTriggerSpeed(Rand::randFloat(.35));
		mGranularPlayer->setRelease(Rand::randFloat(.8));
        mGranularPlayer->trigger();
    }

   // console()<< audio::master()->printGraphToString();

}

void AudioSandboxApp::mouseUp( MouseEvent event )
{
    
    gate = false;
    
    if( doSynthVoice && mSynthVoice ) mSynthVoice->gate(gate);
    if( doSampleVoice && mSampleVoice ) mSampleVoice->gate(gate);
    if( doGranularPlayer && mGranularPlayer ) mGranularPlayer->gate(gate);
    if( doPolySynthVoice && mPolySynthVoice ) mPolySynthVoice->gate(gate);
	if (doPolySamplePlayer && mPolySamplePlayer) mPolySamplePlayer->gate(gate);

}

void AudioSandboxApp::mouseDrag( MouseEvent event )
{

    int note = quantizeMidiNote( mRelMousePos.x * 127, mScale );
    //div.setRate(mCenterDist.x);
    setMouseModulators( event );
    
    if( doGranularPlayer ){

        mGranularPlayer->setPosition( mRelMousePos.x );
        mGranularPlayer->setTriggerSpeed(mRelMousePos.y );
        //mGranularPlayer->setTriggerSpeedJitter( mRelMousePos.y * .01f );
        //mGranularPlayer->setDuration( .45 + mRelMousePos.y * 2 );
        //mGranularPlayer->setPositionJitter( mRelMousePos.y * .1 );

    }

    if( doSynthVoice ) {

        if( note != mOldNote ){

            mSynthVoice->setFreqMidi( float(note) );
            mSynthVoice->trigger();
            //triggerNoteParticles();
        }
    }
	/*
    if( doPolySynthVoice ){
        
        if( note != mOldNote ){
            mPolySynthVoice->setFreqMidi( note );
            triggerNoteParticles();
            mPolySynthVoice->trigger();
        }
        
    }
 */
    mOldNote = note;
}

void AudioSandboxApp::mouseMove( MouseEvent event )
{
    setMouseModulators( event );
    //mDiv.setDivision( ci::clamp(std::floor(mRelMousePos.y*16),0.0f,16.0f) );
    if( mGranularPlayer ){
        mGranularPlayer->setPosition( mRelMousePos.x );
        mGranularPlayer->setDuration(double(.05f+(mRelMousePos.y)));
    }
}

void AudioSandboxApp::fileDrop( FileDropEvent event )
{

    if( doSampleVoice ) mSampleVoice->loadBuffer( event.getFile(0).string() );
    if( doGranularPlayer ) mGranularPlayer->loadBuffer( event.getFile(0).string() );
	if (doPolySamplePlayer) mPolySamplePlayer->loadBuffer(event.getFile(0).string());
}

void AudioSandboxApp::update()
{
    
    mClock.update();
    mDiv.update();
    
    if( gate ){
        if( doGranularPlayer && mGranularPlayer && mGranularPlayer->hasTriggered() ){
            triggerGranularParticles();
        }
    }
    
    mAudioManager->update();
    mParticleSystem->update();
    mNoteParticles->update();
    
}

void AudioSandboxApp::triggerGranularParticles()
{

    if( mGranularPlayer && doGranularPlayer ){
        float rms = mGranularPlayer->getRms();
        Particle* part = mParticleSystem->addParticle();
        part->mPosition = mMousePos;
        part->mVelocity = mMouseVel + Rand::randVec2() * 20.0f * rms;
        part->mRadius = rms * 200;
        part->mColor = vec3(.5f + rms);
    }

}

void AudioSandboxApp::triggerNoteParticles()
{
    
    if( mSynthVoice && doSynthVoice ){
        
        float rms = mSynthVoice->getRms() * 10;
        Particle* part = mNoteParticles->addParticle();
        part->mShape = 1;
        part->mPosition = mMousePos;
        part->mVelocity = mMouseVel + Rand::randVec2() * 30.0f * rms;
        part->mRadius = 30 + rms * 100;
        part->mColor = vec3(rms);
        
    }
    
    if( mPolySynthVoice && doPolySynthVoice ){
        
        float rms = mPolySynthVoice->getRms() ;
        Particle* part = mNoteParticles->addParticle();
        part->mShape = 1;
        part->mPosition = mMousePos;
        part->mVelocity = mMouseVel + Rand::randVec2() * 10.0f * rms;
        part->mRadius = 30 + rms * 100;
        part->mColor = vec3(rms);
        
    }
    
}

void AudioSandboxApp::draw()
{
    
    gl::color( ColorA(0,0,0,.025) );
    gl::drawSolidRect( Rectf( vec2(0,0),app::getWindowSize() ) );
    gl::enableAlphaBlending();
    mParticleSystem->draw();
    mNoteParticles->draw();
    
}

int AudioSandboxApp::quantizeMidiNote( float n, vector<int> s ){
    
    vector<int> fullScale;
    
    fullScale.push_back(s.back()-12);
    fullScale.insert(fullScale.end(),s.begin(),s.end());
    fullScale.push_back(s[0]+12);
    
    int qn = floor(n);
    int interval = qn % 12;
    int octave = qn / 12;
    
    int minDist = 9999;
    int minVal = 9999;
    for( int i=0; i<fullScale.size(); i++ ){
        int dist  = int(abs(interval - fullScale[i])) ;
        if( dist<minDist ){
            minVal = fullScale[i];
            minDist = dist;
        }
    }
    
    interval = minVal;
    return interval + (octave*12);
}


CINDER_APP( AudioSandboxApp, RendererGl )
