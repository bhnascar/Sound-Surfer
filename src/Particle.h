#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofSoundMixer.h"

/* Represents a dynamic on-screen object that moves
 * around based on gravity and forces exerted by other
 * objects. Plays a sound when it makes contact with
 * another object. */
class SoundParticle : public ofxBox2dCircle {
public:
    SoundParticle(float freq);
    ~SoundParticle();
    
    /* Override setup to register audio information. */
    virtual void setup(b2World * b2dworld, float x, float y, float radius);
    
    /* Read-only accessors for private properties. */
    float getFrequency();
    ofVec2f getCurrentPosition();
    ofVec2f getPriorPosition();
    
    /* Standard update/draw callbacks. */
    void update();
    virtual void draw();
    
    static void Initialize(ofSoundMixer* sm);
    
private:
    static ofSoundMixer* sm;
    int soundSourceID;
    float frequency;
    
    ofVec2f currentPosition;
    ofVec2f priorPosition;
};

/* Represents an on-screen object that emits sound
 * when another object passes within its radius of
 * influence. */
class SoundSource : public ofxBox2dCircle {
public:
    SoundSource(float freq);
    ~SoundSource();
    
    /* Read-only accessors for private properties. */
    float getFrequency();
    
    /* Returns true if the particle is within this
     * sound source's radius of influence. */
    bool shouldRepel(SoundParticle& particle);
    
    /* Exerts a force on the particle propertional
     * to its location within this sound source's
     * radius of influence. */
    void repel(SoundParticle& particle);
    
    /* Standard draw callback. */
    virtual void draw(ofColor color);
    
    static void Initialize(ofSoundMixer* sm);
    
private:
    static ofSoundMixer* sm;
    int soundSourceID;
    
    float maxAmplitude;
    float soundRadius;
    float period;
    float frequency;
};

/* Represents a particle source. Emits particles with
 * some frequency. */
class ParticleSource : public ofxBox2dCircle {
public:
    ParticleSource(std::vector<int> pattern);
    ~ParticleSource();
    
    /* Read-only accessors for private properties. */
    int getEmissionCount();
    float getFrequency();
    
    /* Returns true if this particle source should
     * emit a particle again, i.e. interval since
     * last emission is longer than emission frequency. */
    bool shouldEmitParticle();
    
    /* Standard draw callback. */
    virtual void draw();
    
private:
    float emissionFreq = 3;
    float lastEmissionTime = 0;
    int emissionCount;
    
    int patternIndex = 0;
    std::vector<int> frequencyPattern;
};

/* Represents a particle sink. Absorbs particle within
 * a certain radius. */
class ParticleSink : public ofxBox2dCircle {
public:
    ParticleSink(float limit, float freq);
    ~ParticleSink();
    
    /* Read-only accessors for private variables. */
    float getFrequency();
    int getCollectionCount();
    
    /* Attracts a nearby moving particle. Returns true
     * if the particle reaches the sink's location. */
    bool attract(SoundParticle& particle);
    
    /* Returns true if this sink has been filled, i.e.
     * collection count == sink capacity (limit). */
    bool isFull();
    
    /* Plays this particle sink's collection pitch. */
    void play();
    void stop();
    
    /* Standard draw callback. */
    virtual void draw(ofColor color);
    
    static void Initialize(ofSoundMixer* sm);
    
private:
    static ofSoundMixer* sm;
    int soundSourceID;
    bool isPlaying = false;
    
    static ofTrueTypeFont font;
    
    int collectionCount = 0;
    
    float sinkRadius = 100;
    float limit = 10;
    float frequency;
    float period;
};