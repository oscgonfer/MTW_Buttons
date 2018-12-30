#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(backgroundGeneral);
	ofTrueTypeFont::setGlobalDpi(200);

	ofSetVerticalSync(true);
	ofSetFrameRate(30);

	// check for high resolution display //
    if (ofGetScreenWidth()>=2560 && ofGetScreenHeight()>=1600)
    {
        pWidth*=2;
        pFontSize*=2;
	}

}

//--------------------------------------------------------------
void ofApp::draw(){

	std::string file = json_file_name;
	pFont = ofxSmartFont::add("fonts/raleway/Raleway-Medium.ttf", pFontSize*2, "raleway-medium");
	authorFont = ofxSmartFont::add("fonts/raleway/Raleway-Bold.ttf", pFontSize*2, "raleway-bold");

	bool parsingSuccessful = json.open(file);
	int min_index;

	int js_size;

	if (parsingSuccessful) {
		json.getRawString();
		js_size = json.size();

	} else {
		ofLogError("ofApp::draw") << "Failed to parse JSON" << endl;
		js_size = 0;

	}
	
	cout << "[Debug] JSON size " << js_size << endl;
	int posY = 0;

	// Draw rectangle in the background
	ofSetColor(backgroundParagraph);
	ofDrawRectangle(0, 0, posX*1.5 + pWidth, ofGetScreenHeight());

	if (js_size > 0) {
		
		min_index = js_size - tot_num_tweets;
		if (min_index < 0) min_index = 0;
		// cout << "[Debug] We have tweets. min_index = " << min_index << endl;
		
		for (unsigned int i = js_size; i > min_index; i--) {
			if (i==js_size){
				posY = posYinit;
			}
			// cout << "[Debug] index json " << i << endl;
			// cout << json[ofToString(i)] << endl;
			const Json::Value author_full = json[ofToString(i-1)]["author"];
			const Json::Value text_full = json[ofToString(i-1)]["clean_text"];
			// cout << "Tweet # " << ofToString(i) <<  " | Author " << author_full << " | Text " << text_full << endl;
			
			string author = ofToString(author_full);
			author.erase(
			    remove( author.begin(), author.end(), '\"' ), author.end());
			author.erase(
			    remove( author.begin(), author.end(), '\n' ), author.end());

			string text = ofToString(text_full);
			text.erase(
			    remove( text.begin(), text.end(), '\"' ), text.end());
			text.erase(
			    remove( text.begin(), text.end(), '\n' ), text.end());
			text.erase(
			    remove( text.begin(), text.end(), '\t' ), text.end());
			text.erase(
			    remove( text.begin(), text.end(), '\\' ), text.end());

			std::stringstream ss;
			std::stringstream authorString;

			authorString << "@" + ofToString(author) + ": ";
			ss << "@" + ofToString(author) + ": " + ofToString(text);
			cout << ofToString(i) << " " << ss.str() << endl;

			// Author
			ofxParagraph* authorP = new ofxParagraph(authorString.str());
			authorP->setAlignment(ofxParagraph::ALIGN_LEFT);
			authorP->setFont(authorFont);
			authorP->setWidth(pWidth);
			authorP->setColor(pColor);
			authorP->setLeading(pLeading);
			int pSpacing = pFontSize*1;
			authorP->setSpacing(pSpacing);
			authorP->setIndent(0);
			authorP->draw(posX, posY);
			int authorWidth = authorFont->width(authorString.str())+pSpacing;
			
			// Actual tweet
			ofxParagraph* p = new ofxParagraph(text);
			p->setAlignment(ofxParagraph::ALIGN_LEFT);
			p->setFont(pFont);
			p->setWidth(pWidth);
			p->setColor(pColor);
			p->setLeading(pLeading);
			p->setSpacing(pSpacing);
			p->setIndent(authorWidth);
			p->draw(posX, posY);

			// Move down the next paragraph
			int prevHeight = p->getHeight();
			int extraHeight = pLeading*2;
			
			posY += prevHeight + extraHeight;
			

		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case OF_KEY_RIGHT:
			posX ++;
			break; 
		case OF_KEY_LEFT:
			posX --;
			break; 
		case OF_KEY_DOWN:
			posYinit ++;
			break; 
		case OF_KEY_UP:
			posYinit --;
			break; 
		case 'a':
			tot_num_tweets++;
			break;
		case 's':
			tot_num_tweets--;
			break;
		case 'z':
			pWidth++;
			break;
		case 'x':
			pWidth--;
			break;
		case 'q':
			pFontSize++;
			break;
		case 'w':
			pFontSize--;
			break;
		case 'r':
			red++;
			if (red > 255) red = 255;
			break;
		case 't':
			red--;
			if (red < 0) red = 0;
			break;
		case 'g':
			green++;
			if (green > 255) green = 255;
			break;
		case 'h':
			green--;
			if (green < 0) green = 0;
			break;
		case 'b':
			blue++;
			if (blue > 255) blue = 255;
			break;
		case 'n':
			blue--;
			if (blue < 0) blue = 0;
			break;
	}
}