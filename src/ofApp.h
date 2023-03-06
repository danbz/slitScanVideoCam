/*
 Project Title: SlitScanVideoCam
 Description:
 ©Daniel Buzzo 2022, 2023
 dan@buzzo.com
 http://buzzo.com
 https://github.com/danbz
 */

#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"


class circularPixelBuffer{
    // adapted from code by Gil_Fuser https://forum.openframeworks.cc/t/error-regarding-unknown-of-pixel-format/27191

public:
    circularPixelBuffer(){
        currentIndex = 0;
    }
    void setup(int numFrames){
        frames.resize(numFrames);
        currentIndex = numFrames -1;
    }
    void pushPixels(ofPixels& pix){
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = frames.size() -1;
        }
        frames[currentIndex] = pix;
    }
    ofPixels& getDelayedPixels(size_t delay){
        if(delay < frames.size()){
            return frames[ofWrap(delay + currentIndex, 0, frames.size())];
        }
        return frames[0];
    }
    
    int getCurrentIndex() {
        return currentIndex;
    }
    
protected:
    int currentIndex;
    deque<ofPixels> frames;
};

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    // initiate a videograbber object and objects to put image pixel data into
    ofVideoGrabber vidGrabber;
    ofPixels videoPixels, pixels;
    ofTexture videoTexture;
    
    int xSteps, ySteps, scanStyle, speed, numFrames, maxFrames;
    int sWidth, sHeight;
    bool b_radial, b_drawCam, b_smooth, b_timeDirection;
    float currTime, camWidth, camHeight;
    string scanName, time;

    ofTrueTypeFont font;
    //vector< ofPixels > frames;
    circularPixelBuffer frames;
    
   // vector< ofFbo > frames;

    ofImage mask;
    ofFbo maskFBO, layeredFBO;
    
    // face tracker 2
    ofxFaceTracker2 tracker;
};
