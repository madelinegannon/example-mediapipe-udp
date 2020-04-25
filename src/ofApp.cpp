#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetWindowTitle("MediaPipe <--> openFrameworks Example");
    
    // create the socket and bind to port 8080
    ofxUDPSettings settings;
    settings.receiveOn(PORT);
    settings.blocking = false;

    udpConnection.Setup(settings);
    
    
    // initialize wrapper proto object
    wrapper = new ::mediapipe::WrapperHandTracking();
    wrapper->InitAsDefaultInstance();
    
    
    // create an empty hand_pts list with the 21 hand points
    for (int i=0; i<21; i++)
        hand_pts.push_back(ofVec3f());
    
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofSetBackgroundColor(255);
    
    
    // particle system interacting with hand
    setup_particle_system();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // check for incoming messages
    char udpMessage[100000];
    auto n = udpConnection.Receive(udpMessage,100000);
    if (n > 0){
        
        // accept the incoming proto
        wrapper->Clear();
        wrapper->ParseFromArray(udpMessage, udpConnection.GetReceiveBufferSize());
        
        // update the hand_pts list
        for (int i=0; i<wrapper->landmarks().landmark_size(); i++){
            auto& landmark = wrapper->landmarks().landmark(i);
            hand_pts[i] = toOf(landmark.x(), landmark.y(), img_width, img_height);
//            cout << "Landmark " << i << ": " << endl;
//            cout << landmark.DebugString() << endl;
        }
//        cout << wrapper->landmarks().DebugString() << endl;
        
        
        // update the hand rectangle
        if (wrapper->rect().x_center() != 0 && wrapper->rect().y_center() != 0){
        
            hand_rect.rect.position = toOf(wrapper->rect().x_center(), wrapper->rect().y_center(), img_width, img_height);
            ofVec3f temp = toOf(wrapper->rect().width(), wrapper->rect().height(), img_width, img_height);
            hand_rect.rect.width = temp.x;
            hand_rect.rect.height = temp.y;
            hand_rect.rotation = wrapper->rect().rotation();
//            cout << wrapper->rect().DebugString() << endl;
        }
    }
    
    // if we're swatting particles, update
    if (show_particles)
        update_particle_system();

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(225);
    
    draw_debug_hand();
    
    // if we're swatting particles, draw
    if (show_particles)
        draw_particle_system();
    
    ofPushStyle();
    ofSetColor(10);
    ofDrawBitmapString("Press 'SPACE' to push around some particles.", 15, ofGetHeight() - 35);
    ofPopStyle();
}

//--------------------------------------------------------------
ofVec3f ofApp::toOf(float x, float y, int x_bounds, int y_bounds){
    
    // map the normalized points back to the original feed width and height
    return ofVec3f(ofMap(x, 0, 1, 0, img_width), ofMap(y, 0, 1, 0, img_height));
}

//--------------------------------------------------------------
void ofApp::draw_debug_hand(){
    
    
    // Draw and Label the Hand Points
    ofPushStyle();
    ofFill();
    for (int i=0; i<hand_pts.size(); i++){
        ofSetColor(ofColor::black);
        ofDrawBitmapString(ofToString(i), hand_pts[i].x+10, hand_pts[i].y+10);
        ofSetColor(ofColor::cyan, 120);
        ofDrawEllipse(hand_pts[i].x, hand_pts[i].y, 15, 15);
    }
    
    
    // Draw the Hand Skeleton
    /** This is from /mediapipe/graphs/hand_tracking/sub_graphs/renderer_cpu.pbtxt

     # Converts landmarks to drawing primitives for annotation overlay.
     node {
       calculator: "LandmarksToRenderDataCalculator"
       input_stream: "NORM_LANDMARKS:landmarks"
       output_stream: "RENDER_DATA:landmark_render_data"
       node_options: {
         [type.googleapis.com/mediapipe.LandmarksToRenderDataCalculatorOptions] {
           landmark_connections: 0
           landmark_connections: 1
           landmark_connections: 1
           landmark_connections: 2
           landmark_connections: 2
           landmark_connections: 3
           landmark_connections: 3
           landmark_connections: 4
           landmark_connections: 0
           landmark_connections: 5
           landmark_connections: 5
           landmark_connections: 6
           landmark_connections: 6
           landmark_connections: 7
           landmark_connections: 7
           landmark_connections: 8
           landmark_connections: 5
           landmark_connections: 9
           landmark_connections: 9
           landmark_connections: 10
           landmark_connections: 10
           landmark_connections: 11
           landmark_connections: 11
           landmark_connections: 12
           landmark_connections: 9
           landmark_connections: 13
           landmark_connections: 13
           landmark_connections: 14
           landmark_connections: 14
           landmark_connections: 15
           landmark_connections: 15
           landmark_connections: 16
           landmark_connections: 13
           landmark_connections: 17
           landmark_connections: 0
           landmark_connections: 17
           landmark_connections: 17
           landmark_connections: 18
           landmark_connections: 18
           landmark_connections: 19
           landmark_connections: 19
           landmark_connections: 20
           landmark_color { r: 255 g: 0 b: 0 }
           connection_color { r: 0 g: 255 b: 0 }
           thickness: 4.0
         }
       }
     }
     */
    ofSetColor(ofColor::magenta, 120);
    ofSetLineWidth(3);
    
    ofDrawLine(hand_pts[0], hand_pts[1]);
    ofDrawLine(hand_pts[1], hand_pts[2]);
    ofDrawLine(hand_pts[2], hand_pts[3]);
    ofDrawLine(hand_pts[3], hand_pts[4]);
    
    ofDrawLine(hand_pts[0], hand_pts[5]);
    ofDrawLine(hand_pts[5], hand_pts[6]);
    ofDrawLine(hand_pts[6], hand_pts[7]);
    ofDrawLine(hand_pts[7], hand_pts[8]);
    
    ofDrawLine(hand_pts[5], hand_pts[9]);
    ofDrawLine(hand_pts[9], hand_pts[10]);
    ofDrawLine(hand_pts[10], hand_pts[11]);
    ofDrawLine(hand_pts[11], hand_pts[12]);
    
    ofDrawLine(hand_pts[9], hand_pts[13]);
    ofDrawLine(hand_pts[13], hand_pts[14]);
    ofDrawLine(hand_pts[14], hand_pts[15]);
    ofDrawLine(hand_pts[15], hand_pts[16]);
    
    ofDrawLine(hand_pts[13], hand_pts[17]);
    
    ofDrawLine(hand_pts[0], hand_pts[17]);
    ofDrawLine(hand_pts[17], hand_pts[18]);
    ofDrawLine(hand_pts[18], hand_pts[19]);
    ofDrawLine(hand_pts[19], hand_pts[20]);
    

    // Draw the Hand Rectangle
    ofSetLineWidth(1);
    ofSetColor(ofColor::orange);
    ofNoFill();
    ofPushMatrix();
    ofTranslate(hand_rect.rect.position);
    ofRotateRad(hand_rect.rotation);
    ofDrawRectangle(0, 0, hand_rect.rect.width, hand_rect.rect.height);
    ofPopMatrix();
    
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
            show_particles = !show_particles;
            if (show_particles)
                reset_particle_system();
            break;
        case 'r':
            reset_particle_system();
            break;
        case '0':
            currentMode = PARTICLE_MODE_ATTRACT;
            break;
        case '1':
            currentMode = PARTICLE_MODE_REPEL;
            break;
        case '2':
            currentMode = PARTICLE_MODE_NEAREST_POINTS;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::setup_particle_system(){

    int num = 1000;
    p.assign(num, demoParticle());
    currentMode = PARTICLE_MODE_NEAREST_POINTS;

    currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse";

    reset_particle_system();
}

//--------------------------------------------------------------
void ofApp::update_particle_system(){
    // update attractionPoints
    for (int i=0; i<hand_pts.size(); i++){
        attractPoints[i] = hand_pts[i];
    }
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].setMode(currentMode);
        p[i].setAttractPoints(&attractPoints);
        p[i].update();
    }
}

//--------------------------------------------------------------
void ofApp::draw_particle_system(){
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].draw();
    }
}

//--------------------------------------------------------------
void ofApp::reset_particle_system(){

    //these are the attraction points used in the forth demo
    attractPoints.clear();
    for(int i = 0; i < hand_pts.size(); i++){
        attractPoints.push_back( ofPoint( hand_pts[i].x, hand_pts[i].y ) );
    }
    
    attractPointsWithMovement = attractPoints;
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].setMode(currentMode);
        p[i].setAttractPoints(&attractPointsWithMovement);;
        p[i].reset();
    }
}
