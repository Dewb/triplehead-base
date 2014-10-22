#pragma once

#define NUM_SCREENS 6
#define USE_SYPHON

#include "ofMain.h"
#include "ofxUI.h"

#ifdef USE_FENSTER
#include "ofxFenster.h"
#endif

#include "ofxOscReceiver.h"
#include "ofxHapPlayer.h"

#ifdef USE_SYPHON
#include "ofxSyphonServer.h"
#endif

class thbApp : public ofBaseApp {
public:
    thbApp();
    ~thbApp();
    
    void setup();
    void update();
    void draw();
    
    ofFbo& getSingleScreen(int screen);
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
    
    void initBuffer();
    void blankBuffer();
    void initNewMovie(std::string file);
    void bufferMovieFrames(int n);
    void jumpFrames(int n);
    
    ofxUICanvas* _pUI;

#ifdef USE_FENSTER
    ofxFenster* _projectorWindow;
#endif
    
    bool _fullscreen;
    int _radioA;
    int _radioB;
    
    int _nFrameBufferSize;
    int _nFrameDelay;
    int _nFrameLoop;
    float _fFrameAdvance;
    
    float _fFractionalAdvance;
    
    int _nCurrentFrame;
    int _nCurrentLoopStart;
    
    int _nFrameCushion;
    
    float _fHeightPercent;
    float _fHeightOffset;
    int _nMargin;
    
    vector<string> _radioANames;
    vector<string> _radioBNames;
    
    int _bufferCaret;
    vector<ofFbo> _buffer;
    ofVideoPlayer _player;
    
    ofxOscReceiver _osc;

    bool _drawPreview;
    
#ifdef USE_SYPHON
    bool _drawSyphonSingle;
    bool _drawSyphonMultiple;
    
    ofFbo _syphonFrame;
    ofxSyphonServer _syphonServer;
    ofxSyphonServer _syphonScreens[NUM_SCREENS];
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