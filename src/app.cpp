#include "app.h"
#include "utils.h"
#include "ofxFensterManager.h"
#include <stdio.h>
#include <stdlib.h>

#define SIDEBAR_WIDTH 300

#define NUM_SCREENS 12
#define FRAME_RATE_MAX 90
#define FRAME_DELAY_MAX 24
#define FRAME_LOOP_MAX 30
#define FRAME_ADVANCE_MAX 10
#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define FRAME_RATE_MAX_STR STRINGIFY(FRAME_RATE_MAX)
#define FRAME_DELAY_MAX_STR STRINGIFY(FRAME_DELAY_MAX)
#define FRAME_LOOP_MAX_STR STRINGIFY(FRAME_LOOP_MAX)
#define FRAME_ADVANCE_MAX_STR STRINGIFY(FRAME_ADVANCE_MAX)

#define USE_SYPHON

thbApp::thbApp() {
    _pUI = NULL;
    _projectorWindow = NULL;
}

thbApp::~thbApp() {
    if (_pUI)
        delete (_pUI);
    
    if (_projectorWindow) {
        _projectorWindow->destroy();
        ofxFensterManager::get()->deleteFenster(_projectorWindow);
    }    
}

void thbApp::initBuffer() {
    ofPushStyle();
    _nFrameBufferSize = FRAME_DELAY_MAX*(NUM_SCREENS-1) + FRAME_LOOP_MAX + FRAME_ADVANCE_MAX;
    _buffer.clear();
    for(int ii=0; ii<_nFrameBufferSize; ii++) {
        ofFbo fbo;
        fbo.allocate(_player.width, _player.height, GL_RGB);
        fbo.begin();
        ofClear(0,0,0);
        fbo.end();
        _buffer.push_back(fbo);
    }
    ofPopStyle();
    _bufferCaret = 0;
}

void thbApp::blankBuffer() {
    ofPushStyle();
    for(int ii=0; ii<_nFrameBufferSize; ii++) {
        _buffer[ii].begin();
        ofClear(0,0,0);
        _buffer[ii].end();
    }
    ofPopStyle();
}


void thbApp::setup() {
    
    ofxFensterManager::get()->setWindowTitle("Kung Fu Montanez");
    
    ofSetFrameRate(45);
    ofEnableSmoothing();
    
    _fullscreen = false;
    
    _osc.setup(30274);
    
#ifdef USE_SYPHON
    _syphonFrame.allocate(3840, 800, GL_RGB);
    _syphonServer.setName("kfm");
#endif
    
    _nFrameDelay = 7;
    _nFrameLoop = 10;
    _nFrameAdvance = 3;
    
    _nFrameCushion = 0;
    
    _fHeightPercent = 1.0;
    _fHeightOffset = 0.0;
    _nMargin = 0;
        
    loadFile();
        
    initGUI();
}

void thbApp::initGUI() {
    _pUI = new ofxUICanvas(15, 0, SIDEBAR_WIDTH, ofGetHeight());
    _pUI->setWidgetSpacing(5.0);
    _pUI->setDrawBack(false);
    
    _pUI->setFont("GUI/Exo-Regular.ttf", true, true, false, 0.0, OFX_UI_FONT_RESOLUTION);
    _pUI->addWidgetDown(new ofxUILabel("Kung Fu Montanez - @dewb", OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);
     
    _pUI->addWidgetDown(new ofxUILabel("PROJECTORS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Show Window", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("SOURCE VIDEO", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Load...", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("<<", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton(">>", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("PLAYBACK SPEED", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, ofGetFrameRate()/(FRAME_RATE_MAX*1.0),
                                               "FRAME RATE", "0", FRAME_RATE_MAX_STR, OFX_UI_FONT_LARGE));

    char buf[100];
    _pUI->addWidgetDown(new ofxUILabel("DELAY", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, _nFrameDelay/(FRAME_DELAY_MAX*1.0),
                                               "DELAY", "0", FRAME_DELAY_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("LOOP LENGTH", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, _nFrameLoop/(FRAME_LOOP_MAX*1.0),
                                               "LOOP", "0", FRAME_LOOP_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("ADVANCE", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, _nFrameAdvance/(FRAME_ADVANCE_MAX*1.0),
                                               "ADVANCE", "0", FRAME_ADVANCE_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addSpacer(0, 20);

    _pUI->addWidgetDown(new ofxUILabel("HEIGHT", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, 1.0,
                                               "HEIGHT", "0%", "100%", OFX_UI_FONT_LARGE));
    
    _pUI->addWidgetDown(new ofxUILabel("V OFFSET", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, -1.0, 1.0, 0.0,
                                               "V OFFSET", "-1", "1", OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("MARGIN", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 20.0, 0.0,
                                               "MARGIN", "0", "20", OFX_UI_FONT_LARGE));    
    
    ofBackground(255, 20, 32);

    ofAddListener(_pUI->newGUIEvent, this, &thbApp::guiEvent);
}

void thbApp::loadFile() {
    ofFileDialogResult result = ofSystemLoadDialog("Load File", false, "");
    
    if (result.bSuccess) {
        initNewMovie(result.filePath);
    }
    
    // Workaround for ofxFenster modal mouse event bug
    ofxFenster* pWin = ofxFensterManager::get()->getActiveWindow();
    ofxFenster* pDummy = ofxFensterManager::get()->createFenster(0,0,1,1);
    ofxFensterManager::get()->setActiveWindow(pDummy);
    ofxFensterManager::get()->setActiveWindow(pWin);
    ofxFensterManager::get()->deleteFenster(pDummy);
    
}

void thbApp::showProjectorWindow() {
    if (_projectorWindow && _projectorWindow->id != 0) {
        _projectorWindow->destroy();
        ofxFensterManager::get()->deleteFenster(_projectorWindow);
    }
    _projectorWindow = ofxFensterManager::get()->createFenster(400, 300, 750, 200, OF_WINDOW);
    _projectorWindow->addListener(new projectorWindowListener(this));
    _projectorWindow->setWindowTitle("Projector Output");
}

void thbApp::initNewMovie(string file) {
    _player.loadMovie(file);
    
    initBuffer();
    
    _nCurrentFrame = 0;
    _nCurrentLoopStart = 0;
    _bufferCaret = 0;
    bufferMovieFrames(_nFrameLoop+2*_nFrameDelay);
}

void thbApp::bufferMovieFrames(int requestedFrames) {
    int n = requestedFrames;
    //printf("Loading %d frames into buffer at %d (size %d)\n", n, _bufferCaret, _nFrameBufferSize);
    if (n > _nFrameCushion) {
        n -= _nFrameCushion;
        _nFrameCushion = 0;
    } else if (_nFrameCushion > 0) {
        _nFrameCushion -= n;
        return;
    }
    ofPushStyle();
    for (int ii=0; ii < n; ii++) {
        _player.nextFrame();
        _player.update();
        _bufferCaret = (_bufferCaret+1) % _nFrameBufferSize;
        _buffer[_bufferCaret].begin();
        _player.draw(0, 0, _player.width, _player.height);
        _buffer[_bufferCaret].end();
    }
    ofPopStyle();
}

void thbApp::jumpFrames(int n) {
    float pos = (_player.getCurrentFrame() + n) / (_player.getTotalNumFrames() * 1.0);
    if (pos < 0) pos = 0.0;
    if (pos > 1.0) pos = 1.0;
    _player.setPosition(pos);
    _nCurrentFrame = 0;
    _nCurrentLoopStart = 0;
    _bufferCaret = 0;
    bufferMovieFrames(_nFrameLoop + _nFrameDelay * (NUM_SCREENS - 1));
}

void thbApp::update() {
    
    //printf("s: %d e: %d - %d\n", _nCurrentLoopStart, _nCurrentLoopStart + _nFrameLoop, _nCurrentFrame);
    if (_nCurrentFrame == (_nCurrentLoopStart + _nFrameLoop - 1) % _nFrameBufferSize) {
        _nCurrentLoopStart = (_nCurrentLoopStart + _nFrameAdvance) % _nFrameBufferSize;
        _nCurrentFrame = _nCurrentLoopStart;
        bufferMovieFrames(_nFrameAdvance);
    } else {
        _nCurrentFrame = (_nCurrentFrame+1) % _nFrameBufferSize;
    }
    
    while (_osc.hasWaitingMessages()) {
        ofxOscMessage msg;
        if(_osc.getNextMessage(&msg)) {
            string addr = msg.getAddress();
            if (addr == "/montanez/fps") {
                ofSetFrameRate(msg.getArgAsInt32(0));
                updateSlider(_pUI, "FRAME RATE", 0, FRAME_RATE_MAX, ofGetFrameRate());
            } else if (addr == "/montanez/delay") {
                _nFrameDelay = msg.getArgAsInt32(0);
                updateSlider(_pUI, "DELAY", 0, FRAME_DELAY_MAX, _nFrameDelay);
            } else if (addr == "/montanez/loop") {
                _nFrameLoop = msg.getArgAsInt32(0);
                updateSlider(_pUI, "LOOP", 0, FRAME_LOOP_MAX, _nFrameLoop);
            } else if (addr == "/montanez/advance") {
                _nFrameAdvance = msg.getArgAsInt32(0);
                updateSlider(_pUI, "ADVANCE", 0, FRAME_ADVANCE_MAX, _nFrameAdvance);
            }
        }
    }
}

void thbApp::drawProjectorOutput(int w, int h) {
    if (_buffer.size() == 0)
        return;

    ofClear(0,0,0);
    
    int i = 0;
    int N = _nFrameBufferSize;
    int s = NUM_SCREENS;
    
    ofPushStyle();
    for (int i = 0; i < s; i++) {
        auto frame = _buffer[(_nCurrentFrame + i * _nFrameDelay) % N];
        frame.draw(i / (s * 1.0) * w,
                   h * ((1.0 - _fHeightPercent) * 0.5 + _fHeightOffset),
                   (w - _nMargin * (s - 1)) / (s * 1.0),
                   h * _fHeightPercent);
    }    
    ofPopStyle();
}


void thbApp::draw() {
    ofClear(180,0,80);
    _pUI->draw();

#ifdef USE_SYPHON
    _syphonFrame.begin();
    drawProjectorOutput(_syphonFrame.getWidth(), _syphonFrame.getHeight());
    _syphonFrame.end();
    _syphonServer.publishTexture(&(_syphonFrame.getTextureReference()));
#endif
}

void thbApp::keyPressed(int key) {
    
    switch (key) {
        case 'f':
            {
                _fullscreen = !_fullscreen;
                ofSetFullscreen(_fullscreen);
            }
            break;
        case 'l':
            {
                loadFile();
            }
            break;
        case 'p':
            {
                showProjectorWindow();
            }
            break;
    }
}

void thbApp::mousePressed(int x, int y, int button) {
    if (x > SIDEBAR_WIDTH) {
        //_cam.mousePressed(x, y, button);
    }
}

void thbApp::mouseReleased(int x, int y, int button) {
    //_cam.mouseReleased(x, y, button);
}

void thbApp::mouseDragged(int x, int y, int button) {
    //_cam.mouseDragged(x, y, button);
}

void thbApp::mouseMoved(int x, int y) {
}

void thbApp::guiEvent(ofxUIEventArgs &e) {
    string name = e.widget->getName();
    
    if (matchRadioButton(name, _radioANames, &_radioA))
        return;
    if (matchRadioButton(name, _radioBNames, &_radioB))
        return;
    
    if (name == "FRAME RATE") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            ofSetFrameRate(slider->getValue() * FRAME_RATE_MAX);
        }
    } else if (name == "DELAY") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            int newDelay = slider->getValue() * FRAME_DELAY_MAX;
            if (newDelay > _nFrameDelay) {
                bufferMovieFrames(2 * (newDelay - _nFrameDelay));
            } else {
                _nFrameCushion += _nFrameDelay - newDelay;
            }
            _nFrameDelay = newDelay;
        }
    } else if (name == "LOOP") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            int newLoop = slider->getValue() * FRAME_LOOP_MAX;
            if (newLoop > _nFrameLoop) {
                bufferMovieFrames(newLoop - _nFrameLoop);
            } else {
                _nFrameCushion += _nFrameLoop - newLoop;
            }
            _nFrameLoop = newLoop;
        }
    } else if (name == "ADVANCE") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _nFrameAdvance = slider->getValue() * FRAME_ADVANCE_MAX;
        }
    } else if (name == "HEIGHT") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _fHeightPercent = slider->getScaledValue();
        }
    } else if (name == "V OFFSET") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _fHeightOffset = slider->getScaledValue();
        }
    } else if (name == "MARGIN") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _nMargin = slider->getScaledValue();
        }
    } else if (name == "Load...") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            loadFile();
        }
    } else if (name == "<<") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            jumpFrames(-900); // assume roughly 30fps source
        }
    } else if (name == ">>") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            jumpFrames(900); // assume roughly 30fps source
        }
    } else if (name == "Show Window") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            showProjectorWindow();
        }
    }
}

projectorWindowListener::projectorWindowListener(thbApp* app) {
    _app = app;
}

void projectorWindowListener::setup() {
}

void projectorWindowListener::draw() {
    if (_app) {
        int w = ofGetWidth();
        int h = ofGetHeight();
        _app->drawProjectorOutput(w, h);
    }
}