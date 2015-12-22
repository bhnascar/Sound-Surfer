#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "Level.h"
#include "ofSoundMixer.h"

class ofApp : public ofBaseApp {
public:
    ofApp(float width, float height);
    ~ofApp();
    
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(ofMouseEventArgs &e);
    void mousePressed(ofMouseEventArgs &e);
    void mouseReleased(ofMouseEventArgs &e);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void audioOut(float *output, int bufferSize, int nChannels, int deviceID, long unsigned long tickCount);
    
private:    
    /* Current window width, height. */
    float windowWidth;
    float windowHeight;
    
    /* Sound manager. */
    std::shared_ptr<ofSoundMixer> sm;
    
    /* Physics world. */
    ofxBox2d box2d;
    
    /* Current game level. */
    int currentLevelIndex = -1;
    Level* currentLevel;
    
    /* Cumulative player score. */
    ofTrueTypeFont font;
    int score = 0;
    
    /* Instructions */
    ofImage instructions;
    ofImage help;
    bool hkey = false;
    
    /* Generator for game levels. */
    Level* loadNextLevel();
};
