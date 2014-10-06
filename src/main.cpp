#include "app.h"

#ifdef USE_FENSTER
#include "ofxFensterManager.h"
#endif

int main() {
    
    ofSetupOpenGL(1330, 800, OF_WINDOW);
    ofRunApp(new thbApp());
}
