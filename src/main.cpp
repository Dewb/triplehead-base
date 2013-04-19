#include "app.h"
#include "ofxFensterManager.h"

int main() {
    ofSetupOpenGL(ofxFensterManager::get(), 320, 812, OF_WINDOW);
    ofRunFensterApp(new thbApp());
}
