#include "Particle.h"

#include <assert.h>

#define TIME_SCALE 0.01f
#define PIXEL_SCALE 10000.f
#define WAVE_RANGE 200.f
#define WAVE_RANGE_2 100.f

ofSoundMixer* SoundSource::sm = NULL;
ofSoundMixer* SoundParticle::sm = NULL;
ofSoundMixer* ParticleSink::sm = NULL;
ofTrueTypeFont ParticleSink::font;

SoundParticle::SoundParticle(float freq) {
    frequency = freq;
    SMSoundProperties properties;
    properties.freq = frequency;
    properties.volume = 0.f;
    soundSourceID = sm->AddSource(properties);
}

SoundParticle::~SoundParticle() {
    sm->RemoveSource(soundSourceID);
}

void SoundParticle::setup(b2World *b2dworld, float x, float y, float radius) {
    ofxBox2dCircle::setup(b2dworld, x, y, radius);
    
    // Set sound ID as data so we can fetch and play it later in a
    // collision callback. See |Level::onContactStart|.
    setData(&soundSourceID);
}

float SoundParticle::getFrequency() {
    return frequency;
}

ofVec2f SoundParticle::getCurrentPosition() {
    return currentPosition;
}

ofVec2f SoundParticle::getPriorPosition() {
    return priorPosition;
}

void SoundParticle::update() {
    sm->Stop(soundSourceID);
    priorPosition = currentPosition;
    currentPosition = getPosition();
}

void SoundParticle::draw() {
    if(!isBody()) return;
    
    // Translate and rotate context to particle position.
    ofPushMatrix();
    ofTranslate(getPosition().x, getPosition().y, 0);
    ofRotate(getRotation(), 0, 0, 1);
    
    // Draw particle.
    ofCircle(0, 0, getRadius());
    
    // Undo transforms.
    ofPopMatrix();
}

void SoundParticle::Initialize(ofSoundMixer* sm) {
    SoundParticle::sm = sm;
}

SoundSource::SoundSource(float freq)
: frequency(freq) {
    assert(sm != NULL);
    maxAmplitude = 6.f;
    period = 1.f / frequency;
    
    SMSoundProperties properties;
    properties.freq = frequency;
    properties.volume = 0.f;
    soundSourceID = sm->AddSource(properties);
}

SoundSource::~SoundSource() {
    sm->RemoveSource(soundSourceID);
}

float SoundSource::getFrequency() {
    return frequency;
}


bool SoundSource::shouldRepel(SoundParticle& particle) {
    float distance = getPosition().distance(particle.getPosition());
    float freqDiff = particle.getFrequency() - frequency;
    return !(distance > WAVE_RANGE || abs(freqDiff) > 20);
}

void SoundSource::repel(SoundParticle& particle) {
    // Get current distance from rim.
    float distanceFromCenter = getPosition().distance(particle.getCurrentPosition());
    float distanceFromRim = (WAVE_RANGE - distanceFromCenter) / WAVE_RANGE;
    
    // Get prior distance from rim.
    float priorDistanceFromCenter = getPosition().distance(particle.getPriorPosition());
    float priorDistanceFromRim = (WAVE_RANGE - priorDistanceFromCenter) / WAVE_RANGE;
    
    if (priorDistanceFromRim > distanceFromRim) {
        particle.addRepulsionForce(getPosition(), 0.06 * frequency * distanceFromRim);
    }
    else {
        particle.addRepulsionForce(getPosition(), 0.1 * frequency * pow(distanceFromRim, 2));
    }
    sm->Play(soundSourceID, distanceFromRim);
}

void SoundSource::draw(ofColor color) {
    if(!isBody()) return;
    
    // Translate and rotate context to particle position.
    ofPushMatrix();
    ofTranslate(getPosition().x, getPosition().y, 0);
    ofRotate(getRotation(), 0, 0, 1);
    
    // Draw waves.
    ofPushStyle();
    ofNoFill();
    ofSetLineWidth(3);
    float offset = fmod(TIME_SCALE * ofGetElapsedTimef(), period);
    for (float x = offset; x * PIXEL_SCALE < WAVE_RANGE; x += period) {
        float alpha =  (WAVE_RANGE - x * PIXEL_SCALE) / WAVE_RANGE;
        ofSetColor(color.r, color.g, color.b, color.a * alpha);
        ofCircle(0, 0, getRadius() + x * PIXEL_SCALE);
    }
    ofCircle(0, 0, getRadius());
    ofPopStyle();
    
    // Draw particle.
    ofSetColor(color);
    ofCircle(0, 0, getRadius());
    
    // Undo transforms.
    ofPopMatrix();
}

void SoundSource::Initialize(ofSoundMixer* sm) {
    SoundSource::sm = sm;
}

ParticleSource::ParticleSource(std::vector<int> pattern) {
    frequencyPattern = pattern;
}

ParticleSource::~ParticleSource() {
    
}

int ParticleSource::getEmissionCount() {
    return emissionCount;
}

float ParticleSource::getFrequency() {
    float freq = frequencyPattern[patternIndex];
    patternIndex++;
    if (patternIndex >= frequencyPattern.size()) {
        patternIndex = 0;
    }
    return freq;
}

bool ParticleSource::shouldEmitParticle() {
    float now = ofGetElapsedTimef();
    if (now - lastEmissionTime > emissionFreq) {
        emissionCount++;
        lastEmissionTime = now;
        return true;
    }
    return false;
}

void ParticleSource::draw() {
    if(!isBody()) return;
    
    // Translate and rotate context to particle position.
    ofPushMatrix();
    ofTranslate(getPosition().x, getPosition().y, 0);
    ofRotate(getRotation(), 0, 0, 1);
    
    // Draw particle.
    ofFill();
    ofCircle(0, 0, 50);
    
    // Undo transforms.
    ofPopMatrix();
}

ParticleSink::ParticleSink(float limit, float freq)
    : limit(limit), frequency(freq), period(1.f / freq) {
    if (!font.isLoaded()) {
        font.loadFont("Kiddish.ttf", 40, true, true);
    }
    SMSoundProperties properties;
    properties.freq = frequency;
    properties.volume = 0.f;
    soundSourceID = sm->AddSource(properties);
}

ParticleSink::~ParticleSink() {
    sm->RemoveSource(soundSourceID);
}

float ParticleSink::getFrequency() {
    return frequency;
}


int ParticleSink::getCollectionCount() {
    return collectionCount;
}

bool ParticleSink::attract(SoundParticle& particle) {
    float distance = getPosition().distance(particle.getPosition());
    if (abs(particle.getFrequency() - frequency) > 20) {
        return false;
    }
    if (distance > sinkRadius) {
        return false;
    }
    if (distance > 25) {
        particle.addAttractionPoint(getPosition(), 50);
        return false;
    }
    else {
        collectionCount++;
        return true;
    }
}

bool ParticleSink::isFull() {
    return collectionCount >= limit;
}

void ParticleSink::play() {
    isPlaying = true;
    sm->Play(soundSourceID, 1.0);
}

void ParticleSink::stop() {
    isPlaying = false;
    sm->Play(soundSourceID, 0.0);
}

void ParticleSink::draw(ofColor color) {
    if(!isBody()) return;
    
    // Translate and rotate context to particle position.
    ofPushMatrix();
    ofTranslate(getPosition().x, getPosition().y, 0);
    ofRotate(getRotation(), 0, 0, 1);
    
    // Draw waves.
    if (isPlaying) {
        ofPushStyle();
        ofNoFill();
        ofSetLineWidth(3);
        float offset = fmod(TIME_SCALE * ofGetElapsedTimef(), period);
        for (float x = offset; x * PIXEL_SCALE < WAVE_RANGE_2; x += period) {
            float alpha =  (WAVE_RANGE_2 - x * PIXEL_SCALE) / WAVE_RANGE_2;
            ofSetColor(color.r, color.g, color.b, color.a * alpha);
            ofCircle(0, 0, getRadius() + x * PIXEL_SCALE);
        }
        ofCircle(0, 0, getRadius());
        ofPopStyle();
    }
    
    // Draw particle.
    ofPushStyle();
    ofFill();
    ofSetColor(color);
    ofCircle(0, 0, 50);
    ofPopStyle();
    
    // Undo transforms.
    ofPopMatrix();
    
    // Draw collection count.
    ofPushStyle();
    ofSetColor(255, 255, 255, 255);
    std::ostringstream buff;
    buff << (limit - collectionCount);
    int width = font.stringWidth(buff.str());
    int height = font.stringHeight(buff.str());
    font.drawString(buff.str(), getPosition().x - width / 2.f, getPosition().y + height / 2.f);
    ofPopStyle();
}

void ParticleSink::Initialize(ofSoundMixer* sm) {
    ParticleSink::sm = sm;
}