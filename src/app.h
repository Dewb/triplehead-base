#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOscReceiver.h"

#include "kfmPlayer.h"

#ifdef USE_FENSTER
#include "ofxFenster.h"
#endif

class thbApp : public ofBaseApp {
public:
    thbApp();
    ~thbApp();
    
    void setup();
    void update();
    void draw();
    
    void drawProjectorOutput(int p, int x, int y, int w, int h);
    
    void showProjectorWindow();
    void loadFile(int player);
    
    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);

protected:
    void initGUI();
    void guiEvent(ofxUIEventArgs &e);
    
    void jumpFrames(int n);
    
    ofxUICanvas* _pUI;
    
    kfmPlayer players[NUM_PLAYERS];

#ifdef USE_FENSTER
    ofxFenster* _projectorWindow;
#endif
    
    bool _fullscreen;
    
    float _fHeightPercent;
    float _fHeightOffset;
    int _nMargin;
    
    ofxOscReceiver _osc;

    bool _drawPreview;
    
#ifdef USE_SYPHON
    bool _drawSyphonSingle;
    bool _drawSyphonMultiple;
    
    ofFbo _syphonFrame;
    ofxSyphonServer _syphonServer;
#endif
    
    
};

#ifdef USE_FENSTER

class projectorWindowListener : public ofxFensterListener {
public:
    projectorWindowListener(thbApp* app);
    void setup();
    void draw();
protected:
    thbApp* _app;
};

#endif