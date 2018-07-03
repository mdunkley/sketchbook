//
//  ParticleSystem.cpp
//  AudioSandbox
//
//  Created by Michael Dunkley on 3/20/18.
//

#include "ParticleSystem.hpp"

struct greaterThan
{
    template<class T>
    bool operator()(T const &a, T const &b) const { return a > b; }
};


Particle::Particle(){
    mColor = vec3(1,1,1);
    mPosition = vec2(0,0);
    mVelocity = vec2(0,0);
    mRadius = 10;
    mAge = 0.0;
    mLife = 100;
    mShape = 0;
}

Particle::~Particle(){
    
}


ParticleSystem::ParticleSystem(){
    
}

ParticleSystem::~ParticleSystem(){
    
}


void ParticleSystem::draw(){
    
    for( auto p: mParticles ){
        
        gl::color( Color( p->mColor.x,p->mColor.y,p->mColor.z) );
    
        float scaleRampIn = ci::clamp( float(p->mAge/p->mLife*15) ,0.0f,1.0f);
        float scaleRampOut = (1.0-(p->mAge/p->mLife));
        
        float fullScale = p->mRadius * scaleRampIn * scaleRampOut;
        if( p->mShape==0 ) gl::drawSolidCircle( p->mPosition, fullScale );
        else if( p->mShape==1 ) gl::drawSolidRect( Rectf(p->mPosition+vec2(-1,-1)*fullScale ,p->mPosition+vec2(1,1)*fullScale ) );
        //gl::color( ColorA( 1, 1, 1, 1 ) );
        //gl::drawStrokedCircle( p->mPosition, p->mRadius * scaleRampIn * scaleRampOut );
        
    }
}

void ParticleSystem::update(){
    
    int count=0;
    vector<int> kills;
    for( auto p: mParticles ){
        
        p->mPosition += p->mVelocity;
        p->mVelocity *= .98;
        p->mAge += 1;
        
        if( p->mAge>p->mLife ) kills.push_back( count );
            
        count += 1;
    }
    
    std::sort( kills.begin(), kills.end(), greaterThan() );
    
    for( int i=0; i<kills.size(); i++ ){
        
        auto it = mParticles.begin();
        advance( it, kills[i] );
        mParticles.erase( it );
        
    }

}

Particle* ParticleSystem::addParticle(){

    Particle* p = new Particle();
    mParticles.push_back(p);
    return p;
}
