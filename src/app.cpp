#include "app.h"
#include "utils.h"
#include "ofxFensterManager.h"

#define SIDEBAR_WIDTH 320

#define IMAGE_WIDTH 640.0
#define IMAGE_HEIGHT 480.0
#define X_SCALE 100.0
#define Y_SCALE 100.0
#define X_OFFSET -50.0
#define Y_OFFSET -50.0

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

void thbApp::setup() {
    
    ofxFensterManager::get()->setWindowTitle("lorenz attractor photobooth");
    
    ofSetFrameRate(60);
    ofEnableSmoothing();
    glEnable(GL_DEPTH_TEST);
    ofEnableAlphaBlending();
    
    _pixelCoords.resize(IMAGE_WIDTH*IMAGE_HEIGHT);
    initCoords();
    
    loadFile();
    
    _fSpeed = 0.5;
    _fZoom = 0.5;
    _bAlpha = true;
    
    _fullscreen = false;
    
    initGUI();
}

void thbApp::initCoords() {
    for(int jj=0; jj<IMAGE_HEIGHT; jj++) {
        for(int ii=0; ii<IMAGE_WIDTH; ii++) {
            _pixelCoords[jj*IMAGE_WIDTH+ii] = ofVec3f(X_OFFSET + X_SCALE*ii/IMAGE_WIDTH, 0, Y_OFFSET + Y_SCALE*jj/IMAGE_HEIGHT);
        }
    }
}

void thbApp::initGUI() {
    _pUI = new ofxUICanvas(5, 0, SIDEBAR_WIDTH, ofGetHeight());
    _pUI->setWidgetSpacing(5.0);
    _pUI->setDrawBack(true);
    _pUI->setColorBack(ofColor(180, 20, 35));
    
    _pUI->setFont("GUI/Exo-Regular.ttf", true, true, false, 0.0, OFX_UI_FONT_RESOLUTION);
    _pUI->addWidgetDown(new ofxUILabel("lorenz attractor photobooth", OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);
 
    _pUI->addWidgetDown(new ofxUILabel("PROJECTORS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Show Window", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("ACTIONS", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Load Image...", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUILabelButton("Reset", false, 0, 30, 0, 0, OFX_UI_FONT_LARGE));
    _pUI->addSpacer(0, 12);

    _pUI->addWidgetDown(new ofxUILabel("SPEED", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1, _fSpeed, "SPEED", "-4X", "+4X", OFX_UI_FONT_LARGE));

    _pUI->addWidgetDown(new ofxUILabel("ZOOM", OFX_UI_FONT_LARGE));
    _pUI->addWidgetDown(new ofxUIBiLabelSlider(0, 0, SIDEBAR_WIDTH-10, 30, 0, 1, _fZoom, "ZOOM", "min", "max", OFX_UI_FONT_LARGE));

    _pUI->addSpacer(0, 12);
    
    _radioANames.push_back("Points");
    _radioANames.push_back("Blocks");
    _radioANames.push_back("Stretch");
    addRadioAndSetFirstItem(_pUI, "DRAW MODE", _radioANames, OFX_UI_ORIENTATION_VERTICAL, 16, 16);
    _pUI->addSpacer(0, 12);
    
    _radioBNames.push_back("None");
    _radioBNames.push_back("40% Alpha");
    _radioBNames.push_back("Luma is Alpha");
    addRadioAndSetFirstItem(_pUI, "BLEND MODE", _radioBNames, OFX_UI_ORIENTATION_VERTICAL, 16, 16);
    _pUI->addSpacer(0, 12);
   
    ofAddListener(_pUI->newGUIEvent, this, &thbApp::guiEvent);
}

void thbApp::loadFile() {
    ofFileDialogResult result = ofSystemLoadDialog("Load File", false, "");
    
    // Workaround for ofxFenster modal mouse event bug
    ofxFenster* pWin = ofxFensterManager::get()->getActiveWindow();
    ofxFenster* pDummy = ofxFensterManager::get()->createFenster(0,0,1,1);
    ofxFensterManager::get()->setActiveWindow(pDummy);
    ofxFensterManager::get()->setActiveWindow(pWin);
    ofxFensterManager::get()->deleteFenster(pDummy);
    
    if (result.bSuccess) {
        _sourceImage.loadImage(result.filePath);
        _sourceImage.resize(IMAGE_WIDTH, IMAGE_HEIGHT);
    }
}

void thbApp::showProjectorWindow() {
    if (_projectorWindow && _projectorWindow->id != 0) {
        _projectorWindow->destroy();
        ofxFensterManager::get()->deleteFenster(_projectorWindow);
    }
    _projectorWindow = ofxFensterManager::get()->createFenster(400, 100, 800, 600, OF_WINDOW);
    _projectorWindow->addListener(new projectorWindowListener(this));
    _projectorWindow->setWindowTitle("Projector Output");
}

ofVec3f lorenz(ofVec3f pt, float speed) {
    float sigma = 10.0;
    float b = 8.0/3.0;
    float r = 28;
    float dt = 0.01 * speed;
    
    return ofVec3f(pt.x + (sigma * (pt.y - pt.x)) * dt,
                   pt.y + (r * pt.x - pt.y - pt.x * pt.z) * dt,
                   pt.z + (pt.x * pt.y - b * pt.z) * dt
                   );
}

void thbApp::update() {
     for(int jj=0; jj<IMAGE_HEIGHT; jj++) {
        for(int ii=0; ii<IMAGE_WIDTH; ii++) {
            ofVec3f pt = _pixelCoords[jj*IMAGE_WIDTH+ii];
            ofVec3f ptl = lorenz(pt, _fSpeed - 0.5);
            if (std::isnan(ptl.x) || std::isnan(ptl.y) || std::isnan(ptl.z)) {
                //printf("Got NaN\n");
            } else {
                _pixelCoords[jj*IMAGE_WIDTH+ii] = ptl;
            }
        }
    }
}


void thbApp::drawProjectorOutput(int w, int h) {
    ofEnableAlphaBlending();
    glDisable(GL_DEPTH_TEST);

    //glBegin(GL_POINTS);
    for(int jj=0; jj<IMAGE_HEIGHT; jj++) {
        for(int ii=0; ii<IMAGE_WIDTH; ii++) {
            ofColor c = _sourceImage.getColor(ii, jj);
            ofVec3f pt = _pixelCoords[jj*IMAGE_WIDTH+ii];

            if (_radioB == 2) {
                c.a = std::max<unsigned char>(c.r, std::max<unsigned char>(c.g, c.b));
            } else if (_radioB == 1) {
                c.a = 255 * 0.4;
            } else if (_radioB == 0) {
                c.a = 255;
            }

            ofSetColor(c);
            
            /*
            glVertex3f((pt.x - X_OFFSET) / X_SCALE * w,
                       pt.y,
                       (pt.z - Y_OFFSET) / Y_SCALE * h
                       );
            */
            
            pt.x = (pt.x - X_OFFSET) / X_SCALE;
            pt.z = (pt.z - Y_OFFSET) / Y_SCALE;
            
            float scale = 2.0 * std::max<float>(_fZoom, 0.01)/0.5;
            float xshift = 0;
            float yshift = -h/4.0;
            
            float x = 0.5 * w * (1 - 1/scale) + pt.x * w/scale + xshift;
            float y = 0.5 * h * (1 - 1/scale) + pt.z * h/scale + yshift;
            
            if (_radioA == 0) {
                ofRect(x, y, 1, 1);
            } else if (_radioA == 1) {
                ofRect(x, y, 3, 3);
            }
        }
    }
    //glEnd();
}


void thbApp::draw() {
    _pUI->draw();
}

void thbApp::keyPressed(int key) {
    
    switch (key) {
        case 'f':
            {
                _fullscreen = !_fullscreen;
                ofSetFullscreen(_fullscreen);
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
    
    if (name == "SPEED") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _fSpeed = slider->getValue();
        }
    } else if (name == "ZOOM") {
        auto slider = dynamic_cast<ofxUISlider*>(e.widget);
        if (slider) {
            _fZoom = slider->getValue();
        }
    } else if (name == "Load Image...") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue()) {
            loadFile();
        }
    } else if (name == "Reset") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue()) {
            initCoords();
        }
    } else if (name == "Show Window") {
        auto pButton = dynamic_cast<ofxUIButton*>(e.widget);
        if (pButton && !pButton->getValue()) {
            showProjectorWindow();
        }
    } else if (name == "Alpha") {
        auto pButton = dynamic_cast<ofxUIToggle*>(e.widget);
        if (pButton) {
            _bAlpha = pButton->getValue();
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