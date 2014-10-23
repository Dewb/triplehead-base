//
//  kfmPlayer.cpp
//  kfm
//
//  Created by Dewb on 10/22/14.
//
//

#include "kfmPlayer.h"

void kfmPlayer::init(string name) {
    
    _nFrameDelay = 7;
    _nFrameLoop = 10;
    _fFrameAdvance = 3.0;
    
    _fFractionalAdvance = 0.0;
    
    _movieLoaded = false;
    
#ifdef USE_SYPHON
    for (int i = 0; i < NUM_SCREENS; i++) {
        std::ostringstream serverName;
        serverName << "P" << name << " S" << (i + 1);
        _syphonScreens[i].setName(serverName.str());
    }
#endif
}

void kfmPlayer::initBuffer() {
    ofPushStyle();
    _buffer.clear();
    for(int ii = 0; ii < FRAME_BUFFER_SIZE; ii++) {
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

void kfmPlayer::initNewMovie(string file) {
    _player.loadMovie(file);
    
    initBuffer();
    
    _nCurrentFrame = 0;
    _nCurrentLoopStart = 0;
    _bufferCaret = 0;
    bufferMovieFrames(FRAME_LOOP_MAX + FRAME_DELAY_MAX * (NUM_SCREENS - 1));
    
    _movieLoaded = true;
}

void kfmPlayer::blankBuffer() {
    ofPushStyle();
    for(int ii = 0; ii < FRAME_BUFFER_SIZE; ii++) {
        _buffer[ii].begin();
        ofClear(0,0,0);
        _buffer[ii].end();
    }
    ofPopStyle();
}

void kfmPlayer::bufferMovieFrames(int requestedFrames) {
    int n = requestedFrames;
    //printf("Loading %d frames into buffer at %d (size %d)\n", n, _bufferCaret, FRAME_BUFFER_SIZE);
    ofPushStyle();
    for (int ii=0; ii < n; ii++) {
        _player.nextFrame();
        _player.update();
        _bufferCaret = (_bufferCaret+1) % FRAME_BUFFER_SIZE;
        _buffer[_bufferCaret].begin();
        _player.draw(0, 0, _player.getWidth(), _player.getHeight());
        _buffer[_bufferCaret].end();
    }
    ofPopStyle();
}

void kfmPlayer::jumpFrames(int n) {
    if (!_movieLoaded) {
        return;
    }
    float pos = (_player.getCurrentFrame() + n) / (_player.getTotalNumFrames() * 1.0);
    if (pos < 0) pos = 0.0;
    if (pos > 1.0) pos = 1.0;
    _player.setPosition(pos);
    _nCurrentFrame = 0;
    _nCurrentLoopStart = 0;
    _bufferCaret = 0;
    bufferMovieFrames(FRAME_LOOP_MAX + FRAME_DELAY_MAX * (NUM_SCREENS - 1));
}

void kfmPlayer::update() {
    if (!_movieLoaded) {
        return;
    }
    
    //printf("s: %d e: %d - %d\n", _nCurrentLoopStart, _nCurrentLoopStart + _nFrameLoop, _nCurrentFrame);
    if (_nCurrentFrame >= (_nCurrentLoopStart + _nFrameLoop - 1) % FRAME_BUFFER_SIZE) {
        
        _fFractionalAdvance += _fFrameAdvance * (_nFrameLoop / 30.0);
        int advance = floor(_fFractionalAdvance);
        _fFractionalAdvance -= advance;
        //printf("a: %d", advance);
        
        _nCurrentLoopStart = (_nCurrentLoopStart + advance) % FRAME_BUFFER_SIZE;
        _nCurrentFrame = _nCurrentLoopStart;
        bufferMovieFrames(advance);
    } else {
        _nCurrentFrame = (_nCurrentFrame+1) % FRAME_BUFFER_SIZE;
    }
}

ofFbo& kfmPlayer::getSingleScreen(int screen) {
    int index = (_nCurrentFrame + screen * _nFrameDelay) % FRAME_BUFFER_SIZE;
    if (index < 0) index = 0;
    return _buffer[index];
}

void kfmPlayer::draw(int screen, int x, int y, int w, int h) {
    if (!_movieLoaded) {
        return;
    }
    
    auto& frame = getSingleScreen(screen);
    frame.draw(x, y, w, h);
}

void kfmPlayer::publishScreens() {
    if (!_movieLoaded) {
        return;
    }
    for (int i = 0; i < NUM_SCREENS; i++) {
        _syphonScreens[i].publishTexture(&(getSingleScreen(i).getTextureReference()));
    }
}

void kfmPlayer::setDelay(int newDelay) {
    _nFrameDelay = newDelay;
}

void kfmPlayer::setLoop(int newLoop) {
    _nFrameLoop = newLoop;
}

void kfmPlayer::setAdvance(float newAdvance) {
    _fFrameAdvance = newAdvance;
}


