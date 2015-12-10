#pragma once

#include "ofMain.h"

/* Sound modes. */
typedef enum {
    SIN_MODE = 0,
    TRIANGLE_MODE,
    SQUARE_MODE,
    SAW_MODE,
} SMSoundMode;

/* Struct to wrap properties of a sound device. */
struct SMSoundProperties {
    float volume;
    float freq;
};

class ofSoundMixer {
public:
    ofSoundMixer(ofBaseApp* app, int numSources);
    ~ofSoundMixer();
    
    /* Adds a sound source with the given sound properties. 
     * Returns a source ID that can be used to identify and
     * play the sound source using functions below. */
    int AddSource(SMSoundProperties properties);

    /* Removes the sound source with the given source ID. */
    bool RemoveSource(int source);
    
    /* Ping/play/pause. Ping means the sound will be played and then
     * faded out, like a bell. */
    void Ping(int source, float volume, float duration);
    void Play(int source, float volume);
    void Stop(int source);
    
    /* Plays a pitch using the reserved reference source ID */
    void PlayPitch(int pitch);
    
    /* Sets the timbre. */
    void SetMode(SMSoundMode mode);
    
    /* RtAudio callback. */
    void audioOut(float *output, int bufferSize, int nChannels, int deviceID, long unsigned long tickCount);
    
private:
    float SampleSignal(int sourceID, int tick);
    
    ofMutex mutex;
    SMSoundMode mode = SIN_MODE;
    ofSoundStream stream;
    std::vector<SMSoundProperties> sourceProperties;
};

