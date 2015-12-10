#pragma once

#include "ofxBox2d.h"
#include "Particle.h"
#include "ofSoundMixer.h"

class Level
{
public:
    /* Creates a new level. If a filename is provided, the level
     * is prepopulated according to the description in the file. */
    Level(const std::string filename = "");
    ~Level();
    
    /* Loads level from a file. */
    void loadFromFile(const std::string filename);
    
    /* Returns true if the level has been completed. */
    bool complete();
    
    /* Updates all objects in this level. */
    virtual void update();
    
    /* Draws all objects in this level. */
    virtual void draw(bool highlightsOnly = false);
    
    /* Handle key and mouse events. */
    void keyPressed(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(ofMouseEventArgs &e);
    void mousePressed(ofMouseEventArgs &e);
    void mouseReleased(ofMouseEventArgs &e);
    
    static void Initialize(ofxBox2d* box2d, ofSoundMixer* mixer);
    
private:
    /* Shared physics engine. */
    static ofxBox2d* box2d;
    
    /* Shared audio engine. */
    static ofSoundMixer* sm;
    
    /* Shared font for rendering level name. */
    static ofTrueTypeFont font;
    
    /* Drag and drop variables. */
    int mouseX, mouseY;
    ofMutex selectionMutex;
    std::shared_ptr<ofPolyline> currentLine;
    b2Body* selectedBody = NULL;
    
    /* Level play start time. */
    float startTime = -1.f;
    
    /* Level title. */
    std::string title;
    
    /* Level source(s) and sink(s). */
    std::shared_ptr<ParticleSource> source;
    std::shared_ptr<ParticleSink> sink;
    
    /* Level objects. */
    std::vector<std::shared_ptr<ParticleSource> > sources;
    std::vector<std::shared_ptr<ParticleSink> > sinks;
    std::vector<std::shared_ptr<SoundSource> > circles;
    std::vector<std::shared_ptr<SoundParticle> > particles;
    std::vector<std::shared_ptr<ofxBox2dRect> > boxes;
    std::vector<std::shared_ptr<ofxBox2dEdge> > lines;
    
    /* Helper method for converting polyline to box2d edge. */
    ofxBox2dEdge* edgeFromPolyline(const ofPolyline* line);
    
    /* Contact callbacks */
    void onContactStart(ofxBox2dContactArgs &e);
    void onContactEnd(ofxBox2dContactArgs &e);
};
