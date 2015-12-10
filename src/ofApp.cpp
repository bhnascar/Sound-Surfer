#include "ofApp.h"

#define LEVEL_COUNT 6

ofApp::ofApp(float width, float height)
: windowWidth(width), windowHeight(height) {
}

ofApp::~ofApp() {
}

//--------------------------------------------------------------
void ofApp::setup() {
    // Init box2d.
    box2d.init();
    box2d.setGravity(0, 10);
    box2d.setFPS(90.0);
    box2d.enableEvents();
    
    // OpenFramework variables.
    ofSetCircleResolution(50);
    ofSetLineWidth(2.f);
    
    // Init audio system for particles.
    sm = shared_ptr<ofSoundMixer>(new ofSoundMixer(this, 0));
    SoundSource::Initialize(sm.get());
    SoundParticle::Initialize(sm.get());
    
    // Load levels.
    Level::Initialize(&box2d, sm.get());
    currentLevelIndex = 0;
    currentLevel = new Level("level1.txt");
    
    // Load instruction image.
    instructions.loadImage("instructions.png");
    help.loadImage("help.png");
}

//--------------------------------------------------------------
void ofApp::update() {
    box2d.update();
    if (currentLevel->complete()) {
        delete currentLevel;
        currentLevel = loadNextLevel();
    }
    currentLevel->update();
}

//--------------------------------------------------------------
Level* ofApp::loadNextLevel() {
    currentLevelIndex++;
    if (currentLevelIndex >= LEVEL_COUNT) {
        currentLevelIndex = 0;
    }
    std::ostringstream ss;
    ss << "level" << (currentLevelIndex + 1) << ".txt";
    std::string filename = ss.str();
    return new Level(filename);
}

//--------------------------------------------------------------
void ofApp::draw() {
    // Hack to fix mouse disappearance bug.
    ofHideCursor();
    ofShowCursor();
    
    // Draw game level.
    ofBackground(0, 0, 0);
    currentLevel->draw();
    
    // Draw help image if help key is pressed.
    if (hkey) {
        int imageWidth = instructions.getWidth();
        int imageHeight = instructions.getHeight();
        instructions.draw(ofPoint(windowWidth / 2 - imageWidth / 2,
                                  windowHeight / 2 - imageHeight / 2),
                          imageWidth, imageHeight);
    }
    
    // Draw help instructions.
    int imageWidth = help.getWidth();
    int imageHeight = help.getHeight();
    help.draw(ofPoint(20, 20), imageWidth, imageHeight);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    // Pass on key events to level.
    currentLevel->keyPressed(key);
    
    // Do application-level key-handling.
    if (key == 't') {
        // Fullscreen.
        ofToggleFullscreen();
    }
    else if (key == 'h') {
        hkey = true;
    }
    else if (key == 'n') {
        // Skip to next level.
        delete currentLevel;
        currentLevel = loadNextLevel();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    if (key == 'h') {
        hkey = false;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
    // Pass on mouse events to level.
    currentLevel->mouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(ofMouseEventArgs &e) {
    // Pass on mouse events to level.
    currentLevel->mouseDragged(e);
}

//--------------------------------------------------------------
void ofApp::mousePressed(ofMouseEventArgs &e) {
    // Pass on mouse events to level.
    currentLevel->mousePressed(e);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(ofMouseEventArgs &e) {
    // Pass on mouse events to level.
    currentLevel->mouseReleased(e);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) { 

}

//--------------------------------------------------------------
void ofApp::audioOut(float *output, int bufferSize, int nChannels, int deviceID, long unsigned long tickCount) {
    if (sm) {
        sm->audioOut(output, bufferSize, nChannels, deviceID, tickCount);
    }
}
