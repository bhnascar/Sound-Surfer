#include "Level.h"

ofxBox2d* Level::box2d = NULL;
ofSoundMixer* Level::sm = NULL;
ofTrueTypeFont Level::font;

const static string BOX("box");
const static string SOUND("sound");
const static string SOURCE("source");
const static string SINK("sink");

void Level::Initialize(ofxBox2d* b2d, ofSoundMixer* mixer) {
    box2d = b2d;
    sm = mixer;
}

Level::Level(const std::string filename) {
    // Sanity check that box2d has been initialized.
    if (!box2d) {
        std::cerr << "Level::Initialize function must be invoked before creating levels!" << std::endl;
        return;
    }
    
    // Load shared font if it hasn't been loaded.
    if (!font.isLoaded()) {
        font.loadFont("Kiddish.ttf", 40, true, true);
    }
    
    // Load level from filename.
    if (!filename.empty()) {
        loadFromFile(filename);
    }
    
    // Register contact listeners for music playback.
    ofAddListener(box2d->contactStartEvents, this, &Level::onContactStart);
    ofAddListener(box2d->contactEndEvents, this, &Level::onContactEnd);
}

Level::~Level() {
    selectionMutex.lock();
    boxes.clear();
    particles.clear();
    circles.clear();
    selectionMutex.unlock();
}

void Level::loadFromFile(const std::string filename) {
    // Open file.
    std::string currentDirectory = ofDirectory().getAbsolutePath();
    std::string absolutePath = currentDirectory + filename;
    std::ifstream infile(absolutePath.c_str());
    std::string line;
    
    // Get title
    getline(infile, line);
    title = line;
    
    while (getline(infile, line)) {
        // Skip empty lines and comment lines.
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Fetch line prefix.
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;
        
        // Add objects according to instruction from prefix.
        if (prefix == BOX) {
            float x, y, width, height;
            ss >> x >> y >> width >> height;
            boxes.push_back(std::shared_ptr<ofxBox2dRect>(new ofxBox2dRect()));
            boxes.back().get()->setup(box2d->getWorld(), x, y, width, height);
        }
        else if (prefix == SOUND) {
            float x, y, freq;
            ss >> x >> y >> freq;
            circles.push_back(std::shared_ptr<SoundSource>(new SoundSource(freq)));
            circles.back().get()->setup(box2d->getWorld(), x, y, 10);
        }
        else if (prefix == SOURCE) {
            float x, y, freq;
            std::vector<int> emmissionPattern;
            ss >> x >> y;
            while (ss >> freq) {
                emmissionPattern.push_back(freq);
            }
            sources.push_back(std::shared_ptr<ParticleSource>(new ParticleSource(emmissionPattern)));
            sources.back().get()->setup(box2d->getWorld(), x, y, 0);
        }
        else if (prefix == SINK) {
            int limit;
            float x, y, freq;
            ss >> x >> y >> freq >> limit;
            sinks.push_back(std::shared_ptr<ParticleSink>(new ParticleSink(limit, freq)));
            sinks.back().get()->setup(box2d->getWorld(), x, y, 0);
            sinks.back().get()->play();
        }
    }
}

bool Level::complete() {
    for (int i = 0; i < sinks.size(); i++) {
        if (!sinks[i].get()->isFull()) {
            return false;
        }
    }
    return true;
}

void Level::update() {
    // Log start time.
    if (startTime == -1.f) {
        startTime = ofGetElapsedTimef();
    }
    
    // Play preview sounds from sinks.
    for (int i = 0; i < sinks.size(); i++) {
        ParticleSink* sink = sinks[i].get();
        float now = ofGetElapsedTimef();
        if (now - startTime > i && now - startTime < i + 1) {
            sink->play();
        }
        else {
            sink->stop();
        }
    }
    
    // Add new particles.
    for (int i = 0; i < sources.size(); i++) {
        ParticleSource* source = sources[i].get();
        if (source && source->shouldEmitParticle()) {
            float r = 10;
            particles.push_back(std::shared_ptr<SoundParticle>(new SoundParticle(source->getFrequency())));
            particles.back().get()->setPhysics(3.0, 0.53, 0.1);
            particles.back().get()->setup(box2d->getWorld(), source->getPosition().x, source->getPosition().y, r);
        }
    }
    
    // Update all dynamic objects.
    for (int i = 0; i < particles.size(); i++) {
        SoundParticle& particle = *particles[i].get();
        particle.update();
        
        // Delete off-screen particles
        ofVec2f position = particle.getPosition();
        if (position.x < 0 || position.x > ofGetWidth() ||
            position.y > ofGetHeight() + 30) {
            particles.erase(particles.begin() + i);
            i--;
        }
        
        // Repel particles
        std::vector<std::shared_ptr<SoundSource> > repellants;
        for (int j = 0; j < circles.size(); j++) {
            if (circles[j].get()->shouldRepel(particle)) {
                repellants.push_back(circles[j]);
            }
        }
        if (repellants.size() < 2) {
            for (int j = 0; j < repellants.size(); j++) {
                 repellants[j].get()->repel(particle);
            }
        }
        
        // Add attraction force from sinks.
        for (int j = 0; j < sinks.size(); j++) {
            ParticleSink* sink = sinks[j].get();
            if (sink && sink->attract(particle)) {
                particles.erase(particles.begin() + i);
                i--;
            }
        }
    }
}

void Level::draw(bool highlightsOnly) {
    // Draw objects.
    for (int i = 0; i < sinks.size(); i++) {
        float freq = sinks[i].get()->getFrequency() - 220;
        // range is 220 - 880
        float red = ((660.f - freq) / 660.f) * 255;
        float blue = (freq / 660.0f) * 255;
        sinks[i].get()->draw(ofColor(red, 0, blue, 255));
    }
    for (int i = 0; i < sources.size(); i++) {
        ofSetColor(0, 255, 0);
        sources[i].get()->draw();
    }
    for (int i = 0; i < circles.size(); i++) {
        float freq = circles[i].get()->getFrequency() - 220;
        // range is 220 - 880
        float red = ((660.f - freq) / 660.f) * 255;
        float blue = (freq / 660.0f) * 255;
        circles[i].get()->draw(ofColor(red, 0, blue, 255));
    }
    for (int i = 0; i < particles.size(); i++) {
        ofSetColor(255, 255, 255);
        particles[i].get()->draw();
    }
    for (int i = 0; i < boxes.size(); i++) {
        ofSetColor(0, 0, 102);
        boxes[i].get()->draw();
        ofPushStyle();
        ofNoFill();
        ofSetColor(0, 0, 255);
        boxes[i].get()->draw();
        ofPopStyle();
    }
    
    // Draw lines.
    ofSetColor(255, 255, 255);
    if (currentLine) {
        currentLine->draw();
    }
    for (int i = 0; i < lines.size(); i++) {
        lines[i].get()->draw();
    }
    
    // Draw level title.
    ofSetColor(255, 255, 255, 255);
    int width = font.stringWidth(title);
    font.drawString(title, ofGetWidth() / 2.f - width / 2.f, 60);
}

void Level::keyPressed(int key) {
    if (key == 'r') {
        particles.clear();
        lines.clear();
    }
    else if (key == 'u') {
        if (lines.size() > 0) {
            lines.erase(lines.end());
        }
    }
}

void Level::mouseMoved(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void Level::mouseDragged(ofMouseEventArgs &e) {
    selectionMutex.lock();
    if (selectedBody) {
        // If there is a body being dragged, update its position.
        b2Vec2 position(e.x/OFX_BOX2D_SCALE, e.y/OFX_BOX2D_SCALE);
        selectedBody->SetTransform(position, 0);
    }
    if (currentLine) {
        // If there is a line being drawn, add a key point at the mouse
        // position.
        if (currentLine->getVertices().size() < 2) {
            currentLine->addVertex(e.x, e.y);
        }
        else {
            ofPoint& tail = (*currentLine.get())[1];
            tail.x = e.x;
            tail.y = e.y;
        }
    }
    selectionMutex.unlock();
}

void Level::mousePressed(ofMouseEventArgs &e) {
    selectionMutex.lock();
    b2Vec2 position(e.x/OFX_BOX2D_SCALE, e.y/OFX_BOX2D_SCALE);
    
    // Make a small box.
    b2AABB aabb;
    b2Vec2 delta;
    delta.Set(0.001f, 0.001f);
    aabb.lowerBound = position - delta;
    aabb.upperBound = position + delta;
    
    // Query the world for overlapping shapes.
    QueryCallback callback(position);
    box2d->world->QueryAABB(&callback, aabb);
    if (callback.m_fixture) {
        // If there's a hit, set the hit body as the drag body.
        selectedBody = callback.m_fixture->GetBody();
    }
    else {
        // Create a new line.
        currentLine = std::shared_ptr<ofPolyline>(new ofPolyline());
        currentLine->addVertex(e.x, e.y);
    }
    selectionMutex.unlock();
}

void Level::mouseReleased(ofMouseEventArgs &e) {
    selectionMutex.lock();
    selectedBody = NULL;
    if (currentLine) {
        std::shared_ptr<ofxBox2dEdge> edge(edgeFromPolyline(currentLine.get()));
        lines.push_back(edge);
        currentLine.reset();
    }
    selectionMutex.unlock();
}

ofxBox2dEdge* Level::edgeFromPolyline(const ofPolyline* line) {
    ofxBox2dEdge* edge = new ofxBox2dEdge();
    for (int i = 0; i < line->size(); i++) {
        edge->addVertex((*line)[i]);
    }
    edge->create(box2d->getWorld());
    return edge;
}

void Level::onContactStart(ofxBox2dContactArgs &e) {
    if (e.a != NULL) {
        int* soundSourceIDPtrA = (int *)(e.a->GetBody()->GetUserData());
        if (soundSourceIDPtrA) {
            sm->Play(*soundSourceIDPtrA, 1.0);
        }
    }
    if (e.b != NULL) {
        int* soundSourceIDPtrB = (int *)(e.b->GetBody()->GetUserData());
        if (soundSourceIDPtrB) {
            sm->Play(*soundSourceIDPtrB, 1.0);
        }
    }
}

void Level::onContactEnd(ofxBox2dContactArgs &e) {
    // Nothing to do here.
}