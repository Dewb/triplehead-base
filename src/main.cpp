#include "app.h"
#include "ofxFensterManager.h"

int main() {
    ofSetupOpenGL(ofxFensterManager::get(), 330, 700, OF_WINDOW);
    ofRunFensterApp(new thbApp());
}
