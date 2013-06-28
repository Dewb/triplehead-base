#include "app.h"
#include "ofxFensterManager.h"

int main() {
    ofSetupOpenGL(ofxFensterManager::get(), 330, 800, OF_WINDOW);
    ofRunFensterApp(new thbApp());
}
