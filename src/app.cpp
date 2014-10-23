#include "app.h"
#include "utils.h"

#ifdef USE_FENSTER
#include "ofxFensterManager.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define SIDEBAR_WIDTH 300

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

void thbApp::setup() {
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        std::ostringstream name;
        name << (i + 1);
        players[i].init(name.str());
    }
    
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

    _drawSyphonMultiple = true;
    _drawSyphonSingle = false;
#endif
    
    _fHeightPercent = 1.0;
    _fHeightOffset = 0.0;
    _nMargin = 0;
        
    loadFile(0);
        
    initGUI();
}

void thbApp::initGUI() {
    _pUI = new ofxUICanvas(15, 0, SIDEBAR_WIDTH, ofGetHeight());
    _pUI->setWidgetSpacing(5.0);
    _pUI->setDrawBack(false);
    
    _pUI->setFont("GUI/UbuntuMono-B.ttf", true, true, false, 0.0, OFX_UI_FONT_RESOLUTION);
    _pUI->addWidgetDown(new ofxUILabel(" _   ___________  ___ ", OFX_UI_FONT_MEDIUM));
    _pUI->addWidgetDown(new ofxUILabel("| | / /|  ___|  \\/  |   @dewb", OFX_UI_FONT_MEDIUM));
    _pUI->addWidgetDown(new ofxUILabel("| |/ / | |_  | .  . |", OFX_UI_FONT_MEDIUM));
    _pUI->addWidgetDown(new ofxUILabel("|    \\ |  _| | |\\/| |", OFX_UI_FONT_MEDIUM));
    _pUI->addWidgetDown(new ofxUILabel("| |\\  \\| |   | |  | |", OFX_UI_FONT_MEDIUM));
    _pUI->addWidgetDown(new ofxUILabel("\\_| \\_/\\_|   \\_|  |_/", OFX_UI_FONT_MEDIUM));
    
    _pUI->addSpacer(0, 12);
    
#ifdef USE_FENSTER
    _pUI->addWidgetDown(new ofxUILabel("PROJECTORS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Show Window", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);
#endif
    
    _pUI->addWidgetDown(new ofxUILabel("SOURCE VIDEO", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Load 1", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton("Load 2", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton("Load 3", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("<<", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton("<", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton(">", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetRight(new ofxUILabelButton(">>", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("PLAYBACK SPEED", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_RATE_MAX, ofGetFrameRate(),
                                               "FRAME RATE", "0", FRAME_RATE_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("DELAY", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_DELAY_MAX, 7,
                                               "DELAY", "0", FRAME_DELAY_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("LOOP LENGTH", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_LOOP_MAX, 10,
                                               "LOOP", "0", FRAME_LOOP_MAX_STR, OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("ADVANCE", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, FRAME_ADVANCE_MAX, 3.0,
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

void thbApp::loadFile(int player) {
    ofFileDialogResult result = ofSystemLoadDialog("Load File", false, "");
    
    if (result.bSuccess) {
        players[player].initNewMovie(result.filePath);
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


void thbApp::update() {
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].update();
    }
    
    while (_osc.hasWaitingMessages()) {
        ofxOscMessage msg;
        if(_osc.getNextMessage(&msg)) {
            string addr = msg.getAddress();
            if (addr == "/montanez/fps") {
                ofSetFrameRate(msg.getArgAsInt32(0));
                updateSlider(_pUI, "FRAME RATE", 0, FRAME_RATE_MAX, ofGetFrameRate());
            } else if (addr == "/montanez/delay") {
                int newDelay = msg.getArgAsInt32(0);
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setDelay(newDelay);
                }
                updateSlider(_pUI, "DELAY", 0, FRAME_DELAY_MAX, newDelay);
            } else if (addr == "/montanez/loop") {
                int newLoop = msg.getArgAsInt32(0);
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setLoop(newLoop);
                }
                updateSlider(_pUI, "LOOP", 0, FRAME_LOOP_MAX, newLoop);
            } else if (addr == "/montanez/advance") {
                float newAdvance = msg.getArgAsFloat(0);
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setAdvance(newAdvance);
                }
                updateSlider(_pUI, "ADVANCE", 0, FRAME_ADVANCE_MAX, newAdvance);
            }
        }
    }
}

void thbApp::drawProjectorOutput(int p, int x, int y, int w, int h) {
    
    int s = NUM_SCREENS;
    
    ofPushStyle();
    for (int i = 0; i < s; i++) {
        players[p].draw(i,
                   x + i / (s * 1.0) * w,
                   y + h * ((1.0 - _fHeightPercent) * 0.5 + _fHeightOffset),
                   (w - _nMargin * (s - 1)) / (s * 1.0),
                   h * _fHeightPercent);
    }
    ofPopStyle();
}


void thbApp::draw() {
    ofClear(0,0,0);
    _pUI->draw();

    if (_drawPreview) {
        drawProjectorOutput(0, 0, 0, ofGetWidth(), ofGetHeight()/3);
        drawProjectorOutput(1, 0, ofGetHeight()/3, ofGetWidth(), ofGetHeight()/3);
        drawProjectorOutput(2, 0, 2*ofGetHeight()/3, ofGetWidth(), ofGetHeight()/3);
    }
    
#ifdef USE_SYPHON
    if (_drawSyphonSingle) {
        _syphonFrame.begin();
        drawProjectorOutput(0, 0, 0, _syphonFrame.getWidth(), _syphonFrame.getHeight());
        _syphonFrame.end();
        _syphonServer.publishTexture(&(_syphonFrame.getTextureReference()));
    }
    if (_drawSyphonMultiple) {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            players[i].publishScreens();
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
                loadFile(0);
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

void thbApp::jumpFrames(int n) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].jumpFrames(n);
    }
}

void thbApp::guiEvent(ofxUIEventArgs &e) {
    string name = e.widget->getName();
    
    if (name == "FRAME RATE") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            ofSetFrameRate(slider->getScaledValue());
        }
    } else if (name == "DELAY") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            int newDelay = slider->getScaledValue();
            for (int i = 0; i < NUM_PLAYERS; i++) {
                players[i].setDelay(newDelay);
            }
        }
    } else if (name == "LOOP") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            int newLoop = slider->getScaledValue();
            for (int i = 0; i < NUM_PLAYERS; i++) {
                players[i].setLoop(newLoop);
            }
        }
    } else if (name == "ADVANCE") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            float newAdvance = slider->getScaledValue();
            for (int i = 0; i < NUM_PLAYERS; i++) {
                players[i].setAdvance(newAdvance);
            }
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
    } else if (name == "Load 1") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            loadFile(0);
        }
    } else if (name == "Load 2") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            loadFile(1);
        }
    } else if (name == "Load 3") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue())
        {
            loadFile(2);
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