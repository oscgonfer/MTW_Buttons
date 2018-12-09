#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//we run at 60 fps!
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofEnableAntiAliasing();

	//create the socket and bind to port 11999
	ofxUDPSettings settings;
	settings.receiveOn(11999);
	settings.blocking = false;

	udpConnection.Setup(settings);
	ofBackground(0,0,0);
	lengthUdp = 1000;

	// Initialise Stuff
	for (unsigned int row = 0; row< totRows; row++) {
		for (unsigned int column = 0; column < totColumns; column++){
			arrayUpdateTime[row][column] = 0;
		}
	}
	resetTime = 0;
	reset_counter =0;
}

//--------------------------------------------------------------
void ofApp::update(){
	// Status: 1 if pressed, 2 if updated, 0 if nothing
	char udpMessage[lengthUdp];
	udpConnection.Receive(udpMessage, lengthUdp);
	string message=udpMessage;

	string updated;
	string pressed;
	reset_counter = 0;

	if(message!=""){
		vector<string> strPoints = ofSplitString(message,"[/p]");
		int row = 0;
		int column = 0;
		// Check if the message is full (acount for length message)
		if (strPoints.size() == totRows*totColumns + 1){
			for(unsigned int i=0;i<strPoints.size();i++){
				vector<string> point = ofSplitString(strPoints[i],"|");
				row = floor(i/totRows);
				column = i-row*totRows;
				if (point.size() == 2) {
					updated = point[0];
					pressed = point[1];
					if (updated == "1") {
						// cout << "Updating " << i << endl;
						array[row][column] = 2; // We give priority to updated
						arrayUpdateTime[row][column] = ofGetElapsedTimef();
					} else if (pressed == "1") {
						// cout << "Pressing " << i << endl;
						array[row][column] = 1;
						reset_counter ++;
					} else {
						array[row][column] = 0;
					}
				}
			}
		} else if (strPoints.size() == 2) {
			int command = ofToInt(strPoints[0]);
			cout << "[Debug] Received command " << command << endl;
			switch (command) {
				case 49:
					minRectPosx ++;
					break;
				case 50:
					minRectPosx --;
					break;
				case 51:
					minRectPosy ++;
					break;
				case 52:
					minRectPosy --;
					break;
				case 53:
					rectSide ++;
					break;
				case 54:
					rectSide --;
					break;
				case 55:
					rectOffset ++;
					break;
				case 56:
					rectOffset --;
					break;
				case 57:
					thickness ++;
					break;
				case 48:
					thickness --;
					break;
				case 113:
					cout << "minRectPosx " << minRectPosx << " | minRectPosy " << minRectPosy << " | rectSide " << rectSide << " | rectOffset " << rectOffset << " | thickness " << thickness << endl;
					break;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	float alpha;
	float alphaReset;
	float pi = 3.14159;
	float phase = -pi/2;
	float phaseReset = pi/2;
	float n_pulses = 3;
	float n_pulsesReset = 2;
	float now;
	bool reset_status = true;
	now = ofGetElapsedTimef();

	if (reset_counter == totRows*totColumns) {
		if (now - resetTime > timeFadeOutAll) {
			resetTime = ofGetElapsedTimef();
			cout << "[Debug] Reset button pressed " << resetTime << endl;
		}
		reset_status = true;
	} else {
		reset_status = false;
	}

	for (unsigned int i = 0; i < totRows; i ++) {
		for (unsigned int j = 0; j < totColumns; j ++) {

			// Get Alpha for reset
			if (now - resetTime <= timeFadeOutAll) {
				alphaReset = sin(phaseReset+2*pi*(now-resetTime)/(timeFadeOutAll/n_pulsesReset));
			} else {
				alphaReset = 1;
			}

			alphaReset = ofMap(alphaReset, -1, 1, 0, 255);

 			// Get Alpha for button
			if (now - arrayUpdateTime[i][j] < timeFadeOutUpdate) {
				alpha = sin(phase + 2*pi*((now - arrayUpdateTime[i][j])/(timeFadeOutUpdate/n_pulses)));
			} else {
				alpha = -1;
			}

			alpha = ofMap(alpha, -1, 1, 0, 255);

			// Make color square matrix
			int status;
			if (!reset_status) {
				status = array[i][j];
			} else {
				status = 0;
			}

			// Draw under white
			cout << alphaReset << endl;
			ofSetColor(255,255,255,alphaReset);

			// ofNoFill();
			ofRectangle rectW;
			rectW.x = minRectPosx + i*(rectSide + rectOffset);
			rectW.y = minRectPosy + j*(rectSide + rectOffset);
			rectW.width = rectSide;
			rectW.height = rectSide;
			ofDrawRectangle(rectW);

			rectW.x += thickness/2;
			rectW.y += thickness/2;
			rectW.width -= thickness;
			rectW.height -= thickness;
			ofSetColor(0,0,0);
			ofDrawRectangle(rectW);

			if (alphaReset == 255) {
				cout << "hello!" << endl;
				// Draw rectangle for update or press
				// cout << "Row " << i << " Column " << j  << " alpha " << alpha << endl;
				if (status == 1 && reset_status == false) {
					// Pressed, fill black
					ofSetColor(0,0,0);
				} else {
					// Updated, fill with alpha
					ofSetColor(255,255,255,alpha);
				}

				ofRectangle rect;
				rect.x = minRectPosx + i*(rectSide + rectOffset);
				rect.y = minRectPosy + j*(rectSide + rectOffset);
				rect.width = rectSide;
				rect.height = rectSide;

				ofFill();
				ofDrawRectangle(rect);
			}
		}
	}
}