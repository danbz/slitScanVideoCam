/*
 Project Title: SlitScan vodeo cam
 Description: using oF video grabber to sample camera input then accessing pixels in each frame from the camera to create a
 slitscan video.
 each minute we grab the screen to create thumbnail images for the past hour and eacch hour we grab a thumbnail to show images for the last 24 hrs
 
 
 ©Daniel Buzzo 2020, 21, 22, 23
 dan@buzzo.com
 http://buzzo.com
 https://github.com/danbz
 */
#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    // set the width and height of the camera grabber to suit your particular camera
    // common sizes are listed below
    camWidth =  640;  // try to grab at this size from the camera for Raspberry Pi
    camHeight = 480;
    //        camWidth =  1280;  // try to grab at this size from an apple HDwebcam camera.
    //        camHeight = 720;
    
    //    camWidth =  1280;  // try to grab at this size from a standard external 4x3 webcam camera.
    //    camHeight = 1024;
    //    camWidth= 1334; // stereo labs zed camera
    //    camWidth= 1280;
    
    float aspectRatio = camHeight / camWidth;
    
    sWidth = ofGetWidth();
    sHeight = ofGetHeight();
    
    xSteps = ySteps = 0;
    maxFrames = 120;
    numFrames = 60;
    speed = 1;
    scanStyle = 5; // start as push videopush  style
    scanName = "horizontal ribbon";
    b_radial = b_smooth = b_timeDirection = false;
    b_drawCam = false;
    // load a custom truetype font as outline shapes to blend over video layer
    // font.load("LiberationMono-Regular.ttf", 100, true, true, true);
    font.load("m48.TTF", 100, true, true, true);
    
    // ask the video grabber for a list of attached camera devices. & put it into a vector of devices
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    for(int i = 0; i < devices.size(); i++){ // loop through and print out the devices to the console log
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        } else {
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    vidGrabber.setDeviceID(1); // set the ID of the camera we will use
    vidGrabber.setDesiredFrameRate(30); // set how fast we will grab frames from the camera
    vidGrabber.initGrabber(camWidth, camHeight); // set the width and height of the camera
    videoPixels.allocate(camWidth,camHeight, OF_PIXELS_RGBA); // set up our pixel object to be the same size as our camera object
    videoTexture.allocate(camWidth,camHeight, OF_PIXELS_RGBA);
    pixels.allocate(camWidth,camHeight, OF_PIXELS_RGB);
    // ofSetVerticalSync(true);
    ofSetBackgroundColor(0, 0, 0); // set the background colour to dark black
    ofDisableSmoothing();
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    frames.setup(maxFrames);
    
    for (int i = 0; i < maxFrames; i ++){
        //if (vidGrabber.isFrameNew()){
        vidGrabber.update();
        pixels = vidGrabber.getPixels();
        pixels.mirror(false, true);
        //                ofFbo newFBO;
        //                videoTexture.loadData(pixels);
        //                newFBO.begin();
        //                videoTexture.draw(0,0);
        //                newFBO.end();
        //                frames.push_back(newFBO);
        //frames.push_back(pixels);
        frames.pushPixels(pixels);
        //}
    }
    
    maskFBO.allocate(camWidth, camHeight, GL_RGBA);
    layeredFBO.allocate(camWidth, camHeight, GL_RGBA);
    
    maskFBO.begin();
    ofClear(0,0,0,0);
    ofSetColor(255,255,255);
    ofDrawCircle(camWidth/2 , camHeight/2, 200);
    maskFBO.end();
    // face tracker setup
    // set path for tracker data
    //   ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    tracker.setup();
    // tracker.setLandmarkDetectorImageSize(ofGetWidth());
    ofHideCursor;
}


//--------------------------------------------------------------
void ofApp::update(){
    // update the video grabber object
    ofColor color;
    int step, maskX, maskY;
    vidGrabber.update();    //check to see if there is a new frame from the camera to process, and if there is - process it.
    if (vidGrabber.isFrameNew()){
        pixels = vidGrabber.getPixels();
        // fire our new video image to the haar cascade face finder
        //    finder.findHaarObjects(pixels);
        
        // fire the latest image from the webcam to our ofx facetracker
        tracker.update(vidGrabber);
        //  pixels.mirror(false, true);
        // push the new frame to the back of our vector of webcam images
        //        frames.erase(frames.begin());
        //        frames.push_back(pixels);
        // implement basic ring buffer
        frames.pushPixels(pixels);
    }
    
    switch (scanStyle) {
        case 1: // scan horizontal
            for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                color = pixels.getColor( xSteps, y); // get the pixels on line ySteps
                videoPixels.setColor(xSteps, y, color);
            }
            videoTexture.loadData(videoPixels);
            if ( xSteps >= camWidth ) {
                xSteps = 0; // if we are on the bottom line of the image then start at the top again
            }
            xSteps += speed; // step on to the next line. increase this number to make things faster
            break;
            
        case 2: // scan vertical
            for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a line
                color = pixels.getColor(x, ySteps); // get the pixels on line ySteps
                videoPixels.setColor(x, ySteps, color);
            }
            videoTexture.loadData(videoPixels);
            if ( ySteps >= camHeight ) {
                ySteps = 0; // if we are on the bottom line of the image then start at the top again
            }
            ySteps += speed; // step on to the next line. increase this number to make things faster
            break;
            
        case 3: // scan horizontal from centre
            if (xSteps < camWidth){
                for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line to draw new line at 0 in target image
                    color = pixels.getColor( camWidth/2, y); // get the pixels on line ySteps
                    videoPixels.setColor(1, y, color);
                }
                
                for (int x = camWidth; x>=0; x-= 1){
                    for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                        videoPixels.setColor(x, y, videoPixels.getColor( x-1, y )); // copy each pixel in the target to 1 pixel the right
                    }
                }
            }
            videoTexture.loadData(videoPixels);
            xSteps++;
            break;
            
        case 4: // scan vertical from centre
            for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a line to draw new column at 0 in target image
                color = pixels.getColor(x, camHeight/2); // get the pixels on line ySteps
                videoPixels.setColor(x, 1, color);
            }
            
            for (int y = camHeight; y>=0; y-= 1){
                for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a column
                    videoPixels.setColor(x, y, videoPixels.getColor( x, y-1 )); // copy each pixel in the target to 1 pixel below
                }
            }
            videoTexture.loadData(videoPixels);
            break;
            
        case 5: // video version scanStyle in blocks from the frames vector buffer
            
            for (int i =0; i < numFrames; i ++){
                for (int x = xSteps; x < xSteps + camWidth/numFrames; x++){
                    for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                        //  color = frames[maxFrames - numFrames + i].getColor( x, y); // get the pixels on line ySteps
                        // ring buffer version
                        color = frames.getDelayedPixels(  maxFrames - numFrames + i ).getColor( x, y); // get the pixels on line ySteps
                        videoPixels.setColor(x, y, color);
                    }
                }
                xSteps += camWidth/numFrames;
                // BLIT it instead using drawsubsection ?
                // img.drawSubsection(0, 0, 100, 100, mouseX, mouseY);
            }
            videoTexture.loadData(videoPixels);
            xSteps = 0;
            break;
            
        case 6: // make circular layered mask at position of faces and apply mask to buffered image vector.
            // use xy coordinates from haar cascade classifier to centre mask
            step = camWidth / numFrames;
            ofSetColor(255);
            // loop through each frame we are using from the buffer of captured images
            for (int i =0; i < numFrames; i ++){
                // videoTexture.loadData( frames.getDelayedPixels(maxFrames - numFrames + i) );
                if (b_timeDirection){
                    videoTexture.loadData( frames.getDelayedPixels( numFrames - i ) ); // count down from the top (face is now)
                } else {
                    videoTexture.loadData( frames.getDelayedPixels(  i ) ); // count up to the top (face is later)
                }
                // videoTexture.loadData(frames[maxFrames - numFrames + i]);
                maskFBO.begin();
                ofClear(0,0,0,0);
                // walk though each instance of faces found via ofxfacetracker making a mask of reducing size for the final image
                if (tracker.size() >0){
                    for(auto instance : tracker.getInstances() ) {
                        auto rect = instance.getBoundingBox();
                        maskX = rect.x + rect.width/2;
                        maskY = rect.y + rect.height/2 ;
                        // ofDrawRectangle(maskX, maskY, rect.width, rect.height);
                        ofDrawCircle(maskX , maskY, camWidth -  i * (step ) + (rect.width/3));
                        // ofDrawCircle(maskX , maskY, cur.width +  i * step );
                    }
                } else {
                    // no faces found so draw masks from center of screen
                    ofDrawCircle(camWidth/2 , camHeight/2, camWidth -  i * step );
                }
                maskFBO.end();
                // apply the mask we just made to the current frame from the vector of frames
                videoTexture.setAlphaMask( maskFBO.getTexture() );
                // add the masked frame to the buffer of all assembled masked images from the vector of frames
                layeredFBO.begin();
                // draw the current masked frame from the vector into the assembled layered FBO image
                videoTexture.draw(0,0);
                layeredFBO.end();
            }
            // videoTexture.loadData(videoPixels);
            //     videoTexture.setAlphaMask( maskFBO.getTexture() );
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (b_radial){ // draw radial ribbon
        for (int i =0; i<videoTexture.getWidth(); i+=speed){
            ofPushMatrix();
            ofTranslate(sWidth/2, sHeight/2); // centre in right portion of screen
            ofRotateZDeg( ofMap(i, 0, videoTexture.getWidth()/speed, 0, 360));
            videoTexture.drawSubsection(0, 0, speed +2, videoTexture.getHeight(), i, 0);
            ofPopMatrix();
        }
    } else if (scanStyle == 6) { // straight slices
        layeredFBO.draw(0,0,sWidth, sHeight);
    } else {
        videoTexture.draw( 0, 0, sWidth, sHeight); // draw the seconds slitscan video texture we have constructed
    }
    
    if (b_drawCam){ // draw camera debug to screen
        vidGrabber.draw(sWidth-camWidth/4 -10, sHeight-camHeight/4 -10, camWidth/4, camHeight/4); // draw our plain image
        ofDrawBitmapString(" scanning " + scanName + " , 1-scan horiz 2-scan vert 3-ribbon horiz 4-ribbon vert 5-live video, r-radial, c-camview, FPS:" + ofToString(ofGetFrameRate()) + "numFrames: " + ofToString(numFrames) , 10, sHeight -10);
        
        // Draw tracker landmarks
        tracker.drawDebug(0,0,sWidth, sHeight);
        // Draw estimated 3d pose
        // tracker.drawDebugPose(0,0,sWidth, sHeight);
        ofDrawBitmapString( " num of Faces" + ofToString( tracker.size() ), 20, 40);
        //        int maskX, maskY;
        //        for(auto instance : tracker.getInstances() ) {
        //            auto rect = instance.getBoundingBox();
        ////            maskX = rect.x + rect.width/2;
        ////            maskY = rect.y + rect.height/2 ;
        //            ofDrawRectangle(rect);
        //          //  ofDrawCircle(maskX , maskY, camWidth -  i * (step ) + (rect.width/3));
        //            // ofDrawCircle(maskX , maskY, cur.width +  i * step );
        //        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch (key) {
        case 'f':
            ofToggleFullscreen();
            break;
            
        case '1':
            scanStyle = 1;
            scanName = "horizontal";
            break;
        case '2':
            scanStyle = 2;
            scanName = "vertical";
            break;
        case '3':
            scanStyle = 3;
            scanName = "horizontal ribbon";
            break;
            
        case '4':
            scanStyle = 4;
            scanName = "vertical ribbon";
            break;
            
        case '5':
            videoTexture.disableAlphaMask();
            scanStyle = 5;
            scanName = "sliced video";
            break;
            
        case '6':
            scanStyle = 6;
            scanName = "let's make a hole in time";
            break;
            
        case 'c':
            b_drawCam =!b_drawCam;
            break;
            
        case 'r':
            b_radial =!b_radial;
            break;
            
        case 'a':
            if (b_smooth){
                ofEnableSmoothing();
            } else {
                ofDisableSmoothing();
            }
            b_smooth =!b_smooth;
            break;
            
        case '+':
        case '=':
            if (numFrames <maxFrames){
                numFrames ++;
            }
            break;
            
        case '-':
        case '_':
            if (numFrames > 10){
                numFrames --;
            }
            break;
            
        case ' ': //switch timedirection
            b_timeDirection =! b_timeDirection;
            break;
            
            
        default:
            break;
    }
}
