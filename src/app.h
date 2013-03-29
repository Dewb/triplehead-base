#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxFenster.h"

class thbApp : public ofBaseApp {
public:
    thbApp();
    ~thbApp();
    
    void setup();
    void update();
    void draw();
    void drawProjectorOutput(int w, int h);
    
    void showProjectorWindow();
    void loadFile();
    
    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);

protected:
    void initGUI();
    void guiEvent(ofxUIEventArgs &e);
    
    ofxUICanvas* _pUI;
    
    ofxFenster* _projectorWindow;
    
    bool _fullscreen;
    int _radioA;
    int _radioB;
    float _sliderValue;
    vector<string> _radioANames;
    vector<string> _radioBNames;
    
};

class projectorWindowListener : public ofxFensterListener {
public:
    projectorWindowListener(thbApp* app);
    void setup();
    void draw();
protected:
    thbApp* _app;
};
