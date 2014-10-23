#pragma once

#define NUM_PLAYERS 3
#define NUM_SCREENS 8
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


class kfmPlayer {
public:
    
    void init(string name);
    
    void update();
    void draw(int screen, int x, int y, int w, int h);
    ofFbo& getSingleScreen(int screen);
    void publishScreens();
    
    void initBuffer();
    void blankBuffer();
    void initNewMovie(std::string file);
    void bufferMovieFrames(int n);
    void jumpFrames(int n);
    
    
    void setDelay(int delay);
    void setLoop(int loop);
    void setAdvance(float advance);
    
protected:
    
    string name;
    bool _movieLoaded;
    
    int _nFrameDelay;
    int _nFrameLoop;
    float _fFrameAdvance;
    
    float _fFractionalAdvance;
    
    int _nCurrentFrame;
    int _nCurrentLoopStart;
    
    int _nFrameCushion;
    
    int _bufferCaret;
    vector<ofFbo> _buffer;
    ofVideoPlayer _player;

    
#ifdef USE_SYPHON
    ofxSyphonServer _syphonScreens[NUM_SCREENS];
#endif
    
};

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