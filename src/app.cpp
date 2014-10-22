#include "app.h"
#include "utils.h"

#ifdef USE_FENSTER
#include "ofxFensterManager.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define SIDEBAR_WIDTH 300

#define FRAME_RATE_MAX 90
#define FRAME_DELAY_MAX 24
#define FRAME_LOOP_MAX 30
#define FRAME_ADVANCE_MAX 5
#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define FRAME_RATE_MAX_STR STRINGIFY(FRAME_RATE_MAX)
#define FRAME_DELAY_MAX_STR STRINGIFY(FRAME_DELAY_MAX)
#define FRAME_LOOP_MAX_STR STRINGIFY(FRAME_LOOP_MAX)
#define FRAME_ADVANCE_MAX_STR STRINGIFY(FRAME_ADVANCE_MAX)


thbApp::thbApp() {
    _pUI = NULL;
    //_projectorWindow = NULL;
}

thbApp::~thbApp() {
    if (_pUI)
        delete (_pUI);
 
    
#ifdef USE_FENSTER
    if (_projectorWindow) {
        _projectorWindow->destroy();
        ofxFensterManager::get()->deleteFenster(_projectorWindow);
    }    
#endif
}

void thbApp::initBuffer() {
    ofPushStyle();
    _nFrameBufferSize = FRAME_DELAY_MAX*(NUM_SCREENS-1) + FRAME_LOOP_MAX + FRAME_ADVANCE_MAX;
    _buffer.clear();
    for(int ii=0; ii<_nFrameBufferSize; ii++) {
        ofFbo fbo;
        fbo.allocate(_player.getWidth(), _player.getHeight(), GL_RGB);
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
    
#ifdef USE_FENSTER
    ofxFensterManager::get()->setWindowTitle("Kung Fu Montanez");
#endif
    
    ofSetFrameRate(45);
    ofEnableSmoothing();
    
    _fullscreen = false;
    
    _osc.setup(30274);
    
    _drawPreview = true;
    
#ifdef USE_SYPHON
    //_syphonFrame.allocate(3840, 800, GL_RGB);
    //_syphonServer.setName("kfm");

    for (int i = 0; i < NUM_SCREENS; i++) {
        std::ostringstream name;
        name << "Screen " << i;
        _syphonScreens[i].setName(name.str());
    }
    _drawSyphonMultiple = true;
    _drawSyphonSingle = false;
#endif
    
    _nFrameDelay = 7;
    _nFrameLoop = 10;
    _fFrameAdvance = 3.0;
    
    _fFractionalAdvance = 0.0;
    
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
    //_pUI->addWidgetDown(new ofxUILabel("Kung Fu Montanez - @dewb", OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);
     
    _pUI->addWidgetDown(new ofxUILabel("PROJECTORS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Show Window", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("SOURCE VIDEO", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Load...", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("<<", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton("<", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton(">", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton(">>", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("PLAYBACK SPEED", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_RATE_MAX, ofGetFrameRate(),
                                               "FRAME RATE", "0", FRAME_RATE_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("DELAY", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_DELAY_MAX, _nFrameDelay,
                                               "DELAY", "0", FRAME_DELAY_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("LOOP LENGTH", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_LOOP_MAX, _nFrameLoop,
                                               "LOOP", "0", FRAME_LOOP_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("ADVANCE", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_ADVANCE_MAX, _fFrameAdvance,
                                               "ADVANCE", "0", FRAME_ADVANCE_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("HEIGHT", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1.0, 1.0,
                                               "HEIGHT", "0%", "100%", OFX_UI_FONT_LARGE));
    
    _pUI->addWidgetDown(new ofxUILabel("V OFFSET", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, -1.0, 1.0, 0.0,
                                               "V OFFSET", "-1", "1", OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("MARGIN", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 40.0, 0.0,
                                               "MARGIN", "0", "40", OFX_UI_FONT_LARGE));
    
    
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("OUTPUTS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIToggle("PREVIEW", &_drawPreview, 30, 30));
#ifdef USE_SYPHON
    //_pUI->addWidgetRight(new ofxUIToggle("SYPHON-1", &_drawSyphonSingle, 30, 30));
    _pUI->addWidgetRight(new ofxUIToggle("SYPHON-N", &_drawSyphonMultiple, 30, 30));
#endif
    
    ofBackground(255, 20, 32);

    ofAddListener(_pUI->newGUIEvent, this, &thbApp::guiEvent);
}

void thbApp::loadFile() {
    ofFileDialogResult result = ofSystemLoadDialog("Load File", false, "");
    
    if (result.bSuccess) {
        initNewMovie(result.filePath);
    }
    
#ifdef USE_FENSTER
    // Workaround for ofxFenster modal mouse event bug
    ofxFenster* pWin = ofxFensterManager::get()->getActiveWindow();
    ofxFenster* pDummy = ofxFensterManager::get()->createFenster(0,0,1,1);
    ofxFensterManager::get()->setActiveWindow(pDummy);
    ofxFensterManager::get()->setActiveWindow(pWin);
    ofxFensterManager::get()->deleteFenster(pDummy);
#endif
}

void thbApp::showProjectorWindow() {
#ifdef USE_FENSTER
    if (_projectorWindow && _projectorWindow->id != 0) {
        _projectorWindow->destroy();
        ofxFensterManager::get()->deleteFenster(_projectorWindow);
    }
    _projectorWindow = ofxFensterManager::get()->createFenster(400, 300, 750, 200, OF_WINDOW);
    _projectorWindow->addListener(new projectorWindowListener(this));
    _projectorWindow->setWindowTitle("Projector Output");
#endif
}

void thbApp::initNewMovie(string file) {
    _player.loadMovie(file);
    
    initBuffer();
    
    _nCurrentFrame = 0;
    _nCurrentLoopStart = 0;
    _bufferCaret = 0;
    bufferMovieFrames(_nFrameLoop + _nFrameDelay * (NUM_SCREENS - 1));
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
        _player.draw(0, 0, _player.getWidth(), _player.getHeight());
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
    if (_nCurrentFrame >= (_nCurrentLoopStart + _nFrameLoop - 1) % _nFrameBufferSize) {
    
        _fFractionalAdvance += _fFrameAdvance * (_nFrameLoop / 30.0);
        int advance = floor(_fFractionalAdvance);
        _fFractionalAdvance -= advance;
        
        _nCurrentLoopStart = (_nCurrentLoopStart + advance) % _nFrameBufferSize;
        _nCurrentFrame = _nCurrentLoopStart;
        bufferMovieFrames(advance);
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
                _fFrameAdvance = msg.getArgAsFloat(0);
                updateSlider(_pUI, "ADVANCE", 0, FRAME_ADVANCE_MAX, _fFrameAdvance);
            }
        }
    }
}

ofFbo& thbApp::getSingleScreen(int screen) {
    return _buffer[(_nCurrentFrame + screen * _nFrameDelay) % _nFrameBufferSize];
}

void thbApp::drawProjectorOutput(int w, int h) {
    if (_buffer.size() == 0)
        return;

    ofClear(0,0,0,0);
    
    int s = NUM_SCREENS;
    
    ofPushStyle();
    for (int i = 0; i < s; i++) {
        auto& frame = getSingleScreen(i);
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

    if (_drawPreview) {
        drawProjectorOutput(ofGetWidth(), ofGetHeight());
    }
    
#ifdef USE_SYPHON
    if (_drawSyphonSingle) {
        _syphonFrame.begin();
        drawProjectorOutput(_syphonFrame.getWidth(), _syphonFrame.getHeight());
        _syphonFrame.end();
        _syphonServer.publishTexture(&(_syphonFrame.getTextureReference()));
    }
    if (_drawSyphonMultiple) {
        for (int i = 0; i < NUM_SCREENS; i++) {
            std::ostringstream name;
            name << "kfm " << i;
            _syphonScreens[i].publishTexture(&(getSingleScreen(i).getTextureReference()));
        }
    }
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
            ofSetFrameRate(slider->getScaledValue());
        }
    } else if (name == "DELAY") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            int newDelay = slider->getScaledValue();
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
            int newLoop = slider->getScaledValue();
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
            _fFrameAdvance = slider->getScaledValue();
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
    } else if (name == "<") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            jumpFrames(-150); // assume roughly 30fps source
        }
    } else if (name == ">") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            jumpFrames(150); // assume roughly 30fps source
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

#ifdef USE_FENSTER

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

#endif