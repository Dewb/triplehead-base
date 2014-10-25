//
//  utils.cpp
//
//  Created by Michael Dewberry on 3/8/13.
//
//

#include "utils.h"
#include "ofxUI.h"

void addRadioAndSetFirstItem(ofxUICanvas* pUI, string id, vector<string> options, int orientation, int w, int h)
{
    pUI->addRadio(id, options, orientation, w, h);
    {
        // so silly, ofxUI needs to init radios
        auto pToggle = dynamic_cast<ofxUIToggle*>(pUI->getWidget(options[0]));
        if (pToggle)
            pToggle->setValue(true);
    }
}

bool matchRadioButton(string widgetName, vector<string> names, int* pValue)
{
    for (int ii = 0; ii < names.size(); ii++) {
        if (widgetName == names[ii]) {
            if (pValue) {
                *pValue = ii;
            }
            return true;
        }
    }
    return false;
}

void updateSlider(ofxUICanvas* pUI, string widgetName, float min, float max, float value) {
    ofxUISlider* pSlider = dynamic_cast<ofxUISlider*>(pUI->getWidget(widgetName));
    
    if (value < min) { value = min; }
    if (value > max) { value = max; }
    if (pSlider) {
        pSlider->setValue(value);
    }
}

float frand_bounded() {
    return (rand()/(1.0*RAND_MAX)-0.5)*2.0;
}