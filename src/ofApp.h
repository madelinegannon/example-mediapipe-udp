#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "wrapper_hand_tracking.pb.h"
#include "demoParticle.h"

/**
    Example showing how to receive protobuf from separate app over UDP.
    Receiving hand tracking data from google_mediapipe/mediapipe/examples/desktop/hand_tracking on PORT 8080
 */

#define PORT 8080

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);

    
        // Setup UDP COMM
        ofxUDPManager udpConnection;
            
        // This is width/height of input image (webcam)
        // Since we're receiving normalized coordinates,
        // we need the width/height to remap to {x,y} canvas values
        int img_width = 640;
        int img_height = 480;
		
        // Receiving and Drawing the MediaPipe Hand
        ::mediapipe::WrapperHandTracking* wrapper;
        vector<ofVec3f> hand_pts;
        struct HandRect{
            ofRectangle rect;
            float rotation = 0; // in radians
        };
        HandRect hand_rect;
        void draw_debug_hand();
    
        // helper function to convert a normalized point to ofVec3f
        ofVec3f toOf(float x, float y, int x_bounds, int y_bounds);
    
    
    
        // Demo particle system ... just to show how you
        // could use the incoming hand tracking data
        bool show_particles = false;
        particleMode currentMode;
        string currentModeStr;

        vector <demoParticle> p;
        vector <ofPoint> attractPoints;
        vector <ofPoint> attractPointsWithMovement;
        void setup_particle_system();
        void update_particle_system();
        void draw_particle_system();
        void reset_particle_system();
};
