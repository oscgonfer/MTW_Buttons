#include "ofApp.h"
#include "lists.h"

#include <sstream>
#include <string>

//--------------------------------------------------------------

void ofApp::setup(){

	status = TITLE_PRELOAD;

	//Serial
    Serial.listDevices();
    
    vector <ofSerialDeviceInfo> deviceList = Serial.getDeviceList();
    cout <<"Device list size "<< ofToString(deviceList.size())<< endl;
    
    //FIND ARDUINO'S INDEX HERE
    Serial.setup(0, 9600);
    Serial.flush();

    // Some viz stuff
    ofBackground(40,41,35);

    // Init everything to not pressed
    for (int row = 0; row< totRows; row ++) {
    	for (int column = 0; column<totColumns; column++ ){
    		arrayButton[row][column] = false;
    		soundHasBeenPressed[row][column] = false;
    		soundHasBeenReleased[row][column] = true;
    		fadingOut[row][column] = false;
    		timeLeftFadeOut[row][column] = 0;
			timeStartFadeOut[row][column] = 0;
			targetFactor[row][column] = 1;
    	}
    }
}

//--------------------------------------------------------------
void ofApp::update(){

	bool pressed;
	bool istitleplaying;
	bool isambientplaying;
	int indexList;
	float speedAmbient;

	// Read Serial
	int bytesRequired = 33;
	unsigned char bytes[bytesRequired];
	int bytesRemaining = bytesRequired;

	// Serial - loop until we've read everything
	while (bytesRemaining > 0)
	{
		// check for OF_SERIAL_NO_DATA
	  	if ( Serial.available() > 0 ) {
	    // try to read - note offset into the bytes[] array, this is so
	    // that we don't overwrite the bytes we already have
	    int bytesArrayOffset = bytesRequired - bytesRemaining;
	    int result = Serial.readBytes( &bytes[bytesArrayOffset],
	      bytesRemaining );

	    // check for error code
	    if ( result == OF_SERIAL_ERROR ) {
	    	// something bad happened
	    	ofLog( OF_LOG_ERROR, "unrecoverable error reading from serial" );
	      	// bail out
	    	break;
	    } else if ( result == OF_SERIAL_NO_DATA ) {
	    	// nothing was read, try again
	    } else {
	    	// we read some data!
	    	bytesRemaining -= result;
	    }
	  }
	}

	// look for the '\n' character
	int pos1 = 0;
	bool reset = true;
	int sumPressed = 0;
	for (int i = 0; i < bytesRequired; i++) {
        if ((bytes[i]== '\n') && pos1 == 0) {
            pos1 = i;
        }
    }

    // Check who has been pressed
	for (int i = 0; i < 16; i++) {

		int row = floor(i/totRows);
		int column = i-row*totRows;

		pressed = arrayButton[row][column];

		if (bytes[pos1 + i + 1] == '1') {
    		arrayButton[row][column] = true;
    		sumPressed ++;
    	} else {
    		arrayButton[row][column] = false;
     		if (pressed) {
				// timeLvastReleased[row][column] = ofGetElapsedTimef();
				// timeLastPressed[row][column] = ofGetElapsedTimef();
				soundHasBeenPressed[row][column] = true;
			} 
    	}

	}

	if (sumPressed < 8) {
		reset = false;
	}

	// Define reset status
    if (reset) {
    	cout << "RESET" << endl;
    	status = FADE_OUT_AND_STOP; 
    	reset = false;
    }

    // 'State machine'
	switch (status) {
		
		case TITLE_PRELOAD:
		{
			indexTitle = -1;
			indexAmbient = -1;
			soundLoaded = true;

			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					indexList = row * totRows + column;
					arrayButton[row][column] = false;
					soundLoaded = soundLoaded && soundList[row][column].load(pathtoSound + soundListTitle[indexList], true);
					volumeList[row][column] = soundListTitleVol[indexList];
					// No time constraint
					timeLastReleased[row][column] = 0;
					soundHasBeenPressed[row][column] = false;
					soundHasBeenReleased[row][column] = true;

					timeLastPressed[row][column] = 0;
					fadingOut[row][column] = false;
					timeLeftFadeOut[row][column] = 0;
					timeStartFadeOut[row][column] = 0;
					targetFactor[row][column] = 1;

				}
			} 

			if (soundLoaded) {
				cout << "Title loading successful" << endl;
			}

			status = TITLE_LOADED;
			
			break;
		}
		
		case TITLE_LOADED:
		{
			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					pressed = arrayButton[row][column];

					// Time Pressed check

					if (pressed) {
						soundList[row][column].stop();
						soundList[row][column].play();
						timeLastPressed[row][column] = ofGetElapsedTimef();

						soundList[row][column].setVolume(volumeList[row][column]);
						soundList[row][column].setMultiPlay(false);
						soundList[row][column].setLoop(false);

						soundHasBeenReleased[row][column] = false;
						soundHasBeenPressed[row][column] = true;

						indexTitle = row * totRows + column;
						storyNumber = listTitleNumber[indexTitle];
						
						status = AMBIENT_PRELOAD;

						break;
					}
				}
			} 

			break;
		}

		case AMBIENT_PRELOAD:
		{
			// Define list Ambient from title selected
			std::vector<string> listAmbient;
			std::vector<float> listAmbientVol;

			switch (storyNumber){
				case 1:
					listAmbient = listAmbient_T1;
					listAmbientVol = listAmbient_T1Vol;
					break;
				case 2:
					listAmbient = listAmbient_T2;
					listAmbientVol = listAmbient_T2Vol;
					break;
				case 3: 
					listAmbient = listAmbient_T3;
					listAmbientVol = listAmbient_T3Vol;
					break;
				case 4: 
					listAmbient = listAmbient_T4;
					listAmbientVol = listAmbient_T4Vol;
					break;
			}

			soundLoaded = true;

			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					indexList = row * totRows + column;
					
					// Unload everything except the title
					if (indexList!=indexTitle) {
						soundList[row][column].unload();
						// Load ambients
						soundLoaded = soundLoaded && soundList[row][column].load(pathtoSound + listAmbient[indexList], true);	
					
						volumeList[row][column] = listAmbientVol[indexList];				
					}
				}
			} 

			if (soundLoaded) {
				cout << indexTitle << endl;

				cout << "Ambient loading successful" << endl;
			}

			status = AMBIENT_LOADED;
			
			break;
		}

		case AMBIENT_LOADED:
		{
			// Check if title is playing
			rowTitle = floor(indexTitle/totRows);
			colTitle = indexTitle-rowTitle*totRows;

			istitleplaying = soundList[rowTitle][colTitle].isPlaying();

			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					pressed = arrayButton[row][column];

					indexList = row * totRows + column;

					// Time Pressed check
					float lapseOff = ofGetElapsedTimef() - timeLastReleased[row][column];
					float lapseOn = ofGetElapsedTimef() - timeLastPressed[row][column];

					// Title case
					if (indexList == indexTitle) {

						if (!pressed && lapseOff > timeOffMin && soundHasBeenPressed[row][column]) {
							soundHasBeenPressed[row][column] = false;
							soundHasBeenReleased[row][column] = true;
							timeLastReleased[row][column]=ofGetElapsedTimef();
						}

						if (pressed && lapseOn > timeOnMin && soundHasBeenReleased[row][column]) {

							soundList[row][column].stop();
							soundList[row][column].play();
							
							soundList[row][column].setVolume(volumeList[row][column]);
							soundList[row][column].setMultiPlay(false);
							soundList[row][column].setLoop(false);

							timeLastPressed[row][column]=ofGetElapsedTimef();
							soundHasBeenPressed[row][column]=true;
							soundHasBeenReleased[row][column]=false;

							break;
						}

					} else {
						// Trigger ambient
						if (pressed) {
							soundList[row][column].play();
							soundList[row][column].setVolume(volumeList[row][column]);
							soundList[row][column].setLoop(true);
							soundList[row][column].setSpeed(1);

							indexAmbient = indexList;
							ambientHasBeenPressed = true;
							ambientHasBeenReleased = false;
							ambientHasBeenOnceReleased = false;
							status = STORY_PRELOAD;

							break;
						}
					}
				}
			}
			break;
		}
		case STORY_PRELOAD:
		{
			// Define list sounds
			std::vector<string> listSounds;
			std::vector<int> listType;
			std::vector<float> listSoundVol;

			switch (storyNumber){
				case 1:
					listSounds = listSounds_T1;
					listType = listSoundsTypes_T1;
					listSoundVol = listSounds_T1Vol;

					break;
				case 2:
					listSounds = listSounds_T2;
					listType = listSoundsTypes_T2;
					listSoundVol = listSounds_T2Vol;

					break;
				case 3: 
					listSounds = listSounds_T3;
					listType = listSoundsTypes_T3;
					listSoundVol = listSounds_T3Vol;

					break;
				case 4: 
					listSounds = listSounds_T4;
					listType = listSoundsTypes_T4;
					listSoundVol = listSounds_T4Vol;

					break;
			}

			cout << "Selected Story " << storyNumber << endl;

			soundLoaded = true;

			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					indexList = row * totRows + column;

					// Unload everything except the title and ambient
					if (indexList!=indexTitle && indexList!=indexAmbient) {
						soundList[row][column].unload();

						// Load stories
						soundLoaded = soundLoaded && soundList[row][column].load(pathtoSound + listSounds[indexList]);	
						volumeList[row][column] = listSoundVol[indexList];
					
					}
					
					typeList[row][column] = listType[indexList];
					
				}
			}

			if (soundLoaded) {
				cout << "Story loading successful" << endl;
			}
			
			status = STORY_LOADED;
			
			break;
		}

		case STORY_LOADED:
		{

			// Check if title is playing
			rowTitle = floor(indexTitle/totRows);
			colTitle = indexTitle-rowTitle*totRows;

			istitleplaying = soundList[rowTitle][colTitle].isPlaying();

			// Check if ambient is playing
			rowAmbient = floor(indexAmbient/totRows);
			colAmbient = indexAmbient-rowAmbient*totRows;

			isambientplaying = soundList[rowAmbient][colAmbient].isPlaying();
			speedAmbient = soundList[rowAmbient][colAmbient].getSpeed();

			for (int row = 0; row < totRows; row++){
				for (int column = 0; column < totColumns; column++){

					indexList = row * totRows + column;

					pressed = arrayButton[row][column];

					float lapseOff = ofGetElapsedTimef() - timeLastReleased[row][column];
					float lapseOn = ofGetElapsedTimef() - timeLastPressed[row][column];

					// Single or title case
					if (typeList[row][column] == 1 && indexList!=indexAmbient) {

						// Check if pressed or released
						if (!pressed && lapseOff > timeOffMin && soundHasBeenPressed[row][column]){

							soundHasBeenReleased[row][column] = true;
							soundHasBeenPressed[row][column] = false;
							timeLastReleased[row][column] = ofGetElapsedTimef();
						}

						if (pressed && lapseOn > timeOnMin && soundHasBeenReleased[row][column]) {

							soundList[row][column].stop();
							soundList[row][column].play();

							soundList[row][column].setVolume(volumeList[row][column]);
							soundList[row][column].setMultiPlay(false);
	
							soundHasBeenPressed[row][column]=true;
							soundHasBeenReleased[row][column]=false;
							timeLastPressed[row][column] = ofGetElapsedTimef();
							soundList[row][column].setLoop(false);
						}
					}

					// Loop case
					if (typeList[row][column] == 2 && indexList!=indexAmbient && indexList!=indexTitle) {

						if (soundList[row][column].isPlaying()){

							if (pressed) {

								soundList[row][column].setLoop(true);
								fadingOut[row][column] = false;
								soundList[row][column].setVolume(volumeList[row][column]);


							} else {

								// If not pressed, make it fade out
								if (fadingOut[row][column]) {
									// Re calculate time left
									timeLeftFadeOut[row][column] = timeFadeOut - (ofGetElapsedTimef() - timeStartFadeOut[row][column]);
									// Calculate the target volume
									targetFactor[row][column] = timeLeftFadeOut[row][column]/timeFadeOut;

									
									if (targetFactor[row][column]*volumeList[row][column] < 0.01f) {
										soundList[row][column].stop();
										fadingOut[row][column] = false;
									} else {
										soundList[row][column].setVolume(targetFactor[row][column]*volumeList[row][column]);
									}

								} else {

									fadingOut[row][column] = true;
									timeStartFadeOut[row][column] = ofGetElapsedTimef();

								}
								
								// If it's a loop sound, let it finish once is not pressed
								soundList[row][column].setLoop(false);

							}

						} else {

							fadingOut[row][column] = false;
							if (pressed) {
								soundList[row][column].play();
								soundList[row][column].setVolume(volumeList[row][column]);
								soundList[row][column].setLoop(true);
							}

						}
					}

					if (indexList == indexAmbient){
						// Case when ambient is done on it's own
						if (pressed && !isambientplaying) {
							soundList[row][column].play();
							soundList[row][column].setVolume(volumeList[row][column]);
							soundList[row][column].setMultiPlay(false);
							soundList[row][column].setSpeed(1.0f);
						}

						// Check when ambient has been released after pressed
						if (!pressed && ambientHasBeenPressed){
							// Ambient has been released
							ambientHasBeenReleased = true;
							ambientHasBeenPressed = false;

							if (!ambientHasBeenOnceReleased) {
								ambientHasBeenOnceReleased = true;
							}

							ambientReleaseTime = ofGetElapsedTimef();
						}

						// Check when ambient has been pressed after released
						if (pressed && ambientHasBeenReleased) {
							ambientHasBeenPressed = true;
							ambientHasBeenReleased = false;
							ambientPressTime = ofGetElapsedTimef();
						}

						// Change speed of ambient in case of release
						if (!pressed && ambientHasBeenReleased && speedAmbient > 1 && isambientplaying) {
							float newSpeedAmbient = fmax(1, targetSpeedAmbient - (ofGetElapsedTimef() - ambientReleaseTime)/timeTransientSpeedAmbient);
							// Put speed back to normal
							soundList[row][column].setSpeed(newSpeedAmbient);
						}

						// Change speed of ambient in case of re-press
						if (pressed && ambientHasBeenPressed && isambientplaying && ambientHasBeenOnceReleased) {
							float newSpeedAmbient = fmin(targetSpeedAmbient, (ofGetElapsedTimef()-ambientPressTime)/timeTransientSpeedAmbient + 1);
							// Set the desired speed
							soundList[row][column].setSpeed(newSpeedAmbient);
						}
					}
				}
			}

			break;
		}

		case FADE_OUT_AND_STOP:
		{
			float timeLeftFadeOutAll = 0;
			float timeStartFadeOutAll = 0;
			float targetFactorAll = 1;
			bool actuallyReset = false;
			
			// Set time left for fade out as max time for fade out
			timeLeftFadeOutAll = timeFadeOutAll;
			// Get time at which we will start fading out
			timeStartFadeOutAll = ofGetElapsedTimef(); 
			for (int i=0;i<totColumns;i++){
				for (int j=0;j<totRows;j++){
					if (timeLastReleased[j][i] > 0) {
						actuallyReset = true;
					}
				}
			}

			if (actuallyReset) {

				while (timeLeftFadeOutAll>0) {
					// Re calculate time left
					timeLeftFadeOutAll = timeFadeOutAll - (ofGetElapsedTimef() - timeStartFadeOutAll);
					// Calculate the target volume
					targetFactorAll = fmin(1, timeLeftFadeOutAll/timeFadeOutAll);

					for (int row = 0; row < totRows; row++){
						for (int column = 0; column < totColumns; column++){
							soundList[row][column].setLoop(false);
							soundList[row][column].setSpeed(1);
							
							if (targetFactorAll*volumeList[row][column] < 0.01) {
								soundList[row][column].stop();
							} else {
								soundList[row][column].setVolume(targetFactorAll*volumeList[row][column]);
							}
							
							timeLastReleased[row][column] = 0;

						}
					}
				}
			}


    		// Serial.flush();

			status = TITLE_PRELOAD;

			break;
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){

	if (viewGraph) {
		for (int i = 0; i<totRows; i++){
			for (int j = 0; j<totColumns; j++) {

				bool pressed = arrayButton[j][i];

				if (pressed) {
					ofSetColor(248,35,114);
				} else {
					ofSetColor(235,237,239);
				}
	    		ofFill();

				ofRectangle rect;

	    		rect.x = minRectPos + i*(rectSide+rectOffset);
	    		rect.y = minRectPos + j*(rectSide+rectOffset);
	    		rect.width = rectSide;
	    		rect.height = rectSide;

	    		ofDrawRectangle(rect);

	    		// Text in center if is playing
	    		int textOnX = rect.x + rectSide/2;
	    		int textOnY = rect.y + rectSide/2;

	    		bool playing;
	    		playing = soundList[j][i].isPlaying();
				ofSetColor(40,41,35);
	    		stringstream text;
	    		if (playing) {
	    			text << "ON" << endl;
	    		} else {
	    			text << "OFF" << endl;
	    		}

	            ofDrawBitmapString(text.str(), textOnX, textOnY);

	            // Text below for time last released
	            int textReleasedX = rect.x + rectSide/10;
	    		int textReleasedY = rect.y + rectSide/10; 

	    		float time = timeLastReleased[j][i];
	    		stringstream textTime;
	    		textTime << "Time Rel: " << time << endl;

				ofDrawBitmapString(textTime.str(), textReleasedX, textReleasedY);

				// Text below for speed
	            int textSpeedX = rect.x + rectSide/10;
	    		int textSpeedY = rect.y + 8*rectSide/10; 

				float speed = 0;
	    		speed = soundList[j][i].getSpeed();
	    		stringstream textSpeed;
	    		textSpeed << "Speed: " << speed << endl;

				ofDrawBitmapString(textSpeed.str(), textSpeedX, textSpeedY);
			}
		}
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key) {
        case 'v':
        	if (!viewGraph){
        		viewGraph = true;
        	} else {
        		viewGraph = false;
        	}
            break;
    }
}
