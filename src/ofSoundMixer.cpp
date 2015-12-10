#include "ofSoundMixer.h"

#define SAMPLING_RATE 44100

ofSoundMixer::ofSoundMixer(ofBaseApp* app, int numSources) {
    for (int i = 0; i < numSources; i++) {
        // Create sound properties
        SMSoundProperties properties;
        properties.volume = 0.f;
        properties.freq = 770.f - i * 110.f;
        sourceProperties.push_back(properties);
    }
    
    // Create sound stream
    stream.setup(app, 2, 0, 44100, 1024, 1);
}

ofSoundMixer::~ofSoundMixer() {
    stream.stop();
    stream.close();
}

int ofSoundMixer::AddSource(SMSoundProperties properties) {
    sourceProperties.push_back(properties);
    return sourceProperties.size() - 1;
}

bool ofSoundMixer::RemoveSource(int source) {
    sourceProperties[source].volume = 0;
}

void ofSoundMixer::Ping(int source, float volume, float duration) {
    if (source < 0 || source >= sourceProperties.size()) {
        std::cerr << "Invalid source ID (Ping)!" << std::endl;
        return;
    }
    mutex.lock();
    sourceProperties[source].volume = ofLerp(volume, 1, 0.8);
    mutex.unlock();
}

void ofSoundMixer::Play(int source, float volume) {
    if (source < 0 || source >= sourceProperties.size()) {
        std::cerr << "Invalid source ID (Play)!" << std::endl;
        return;
    }
    mutex.lock();
    sourceProperties[source].volume = ofLerp(max(volume, sourceProperties[source].volume), 0, 0.8);
    mutex.unlock();
}

void ofSoundMixer::Stop(int source) {
    if (source < 0 || source >= sourceProperties.size()) {
        std::cerr << "Invalid source ID (Stop)!" << std::endl;
        return;
    }
    mutex.lock();
    sourceProperties[source].volume = ofLerp(sourceProperties[source].volume, 0, 0.1);
    mutex.unlock();
}

void ofSoundMixer::SetMode(SMSoundMode mode) {
    this->mode = mode;
}

float ofSoundMixer::SampleSignal(int sourceID, int tick) {
    float adjustedTick = (float)tick / SAMPLING_RATE;
    float volume = sourceProperties[sourceID].volume;
    float freq = sourceProperties[sourceID].freq;
    float period = 1.f / freq;
    switch (mode) {
        case SIN_MODE:
            return volume * sin(2.f * M_PI * freq * adjustedTick);
        case SAW_MODE:
        case TRIANGLE_MODE:
            return volume * (2.f * freq * (fmod(adjustedTick, period) - 0.5f * period));
        case SQUARE_MODE:
            return volume * (fmod(adjustedTick, period) <= 0.5f * period ? 1 : -1);
        default:
            return 0.f;
    }
}

void ofSoundMixer::audioOut(float *output, int bufferSize, int nChannels, int deviceID, long unsigned long tickCount) {
    mutex.lock();
    static int tick = 0;
    for(int i = 0; i < bufferSize * nChannels; i += nChannels) {
        int activeSourceCount = 0;
        float audioSample = 0.f;
        for (int j = 0; j < sourceProperties.size(); j++) {
            if (sourceProperties[j].volume > 0.f) {
                activeSourceCount++;
                audioSample += SampleSignal(j, tick);
            }
        }
        audioSample /= activeSourceCount;
        tick++;
        for (int j = 0; j < nChannels; j++) {
            output[i + j] = audioSample;
        }
    }
    mutex.unlock();
}