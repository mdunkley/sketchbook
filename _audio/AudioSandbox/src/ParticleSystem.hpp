//
//  ParticleSystem.hpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/20/18.
//

#ifndef ParticleSystem_hpp
#define ParticleSystem_hpp

#include <stdio.h>
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"
#include "cinder/audio/Context.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include "Instrument.hpp"

using namespace ci;
using namespace std;


class Particle {
public:
    
    Particle();
    Particle( vec2 pos, vec2 vel );
    ~Particle();
    
    vec2 mPosition;
    vec2 mVelocity;
    vec3 mColor;
    
    float mRadius;
    float mAge;
    float mLife;
    
    int mShape; // cicle, square
    
    Instrument* instrument;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    void update();
    void draw();
    Particle* addParticle();
private:
    
    list<Particle*> mParticles;
    
};


#endif /* ParticleSystem_hpp */
