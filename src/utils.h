//
//  utils.h
//
//  Created by Michael Dewberry on 3/8/13.
//
//

#include <list>
#include <vector>
#include <string>

using std::list;
using std::vector;
using std::string;

class ofxUICanvas;

template <typename T>
void deletePointerCollection(std::vector<T*> v) {
    for (auto iter = v.begin(); iter != v.end(); iter++)
        delete *iter;
    v.clear();
}

template <typename T>
void deletePointerCollection(std::list<T*> v) {
    for (auto iter = v.begin(); iter != v.end(); iter++)
        delete *iter;
    v.clear();
}

void addRadioAndSetFirstItem(ofxUICanvas* pUI, std::string id, vector<string> options, int orientation, int w, int h);
bool matchRadioButton(string widgetName, vector<string> names, int* pValue);
void updateSlider(ofxUICanvas* pUI, string widgetName, float min, float max, float value);

float frand_bounded();
