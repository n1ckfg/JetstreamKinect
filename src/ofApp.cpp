#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
    settings.loadFile("settings.xml");
    
    ofSetVerticalSync(false);
    ofHideCursor();

    appFramerate = settings.getValue("settings:app_framerate", 60);
    camFramerate = settings.getValue("settings:cam_framerate", 30);
    ofSetFrameRate(appFramerate);
	camId = settings.getValue("settings:cam_id", 0);
	
    syncVideoQuality = settings.getValue("settings:osc_video_quality", 3); 
    videoColor = (bool) settings.getValue("settings:video_color", 0); 
    
    width = settings.getValue("settings:width", 640);
    height = settings.getValue("settings:height", 480);
    ofSetWindowShape(width, height);

    debug = (bool) settings.getValue("settings:debug", 1);

    sendOsc = (bool) settings.getValue("settings:send_osc", 1); 
    sendWs = (bool) settings.getValue("settings:send_ws", 1); 
    sendHttp = (bool) settings.getValue("settings:send_http", 1); 
    sendMjpeg = (bool) settings.getValue("settings:send_mjpeg", 1); 
    
    syncVideo = (bool) settings.getValue("settings:sync_video", 0); 
    blobs = (bool) settings.getValue("settings:blobs", 1);
    contours = (bool) settings.getValue("settings:contours", 0); 
    contourSlices = settings.getValue("settings:contour_slices", 10); 
    brightestPixel = (bool) settings.getValue("settings:brightest_pixel", 0); 

    oscHost = settings.getValue("settings:osc_host", "127.0.0.1");
    oscPort = settings.getValue("settings:osc_port", 7110);
    streamPort = settings.getValue("settings:stream_port", 7111);
    wsPort = settings.getValue("settings:ws_port", 7112);
    postPort = settings.getValue("settings:post_port", 7113);

    debug = (bool) settings.getValue("settings:debug", 1);
    rpiCamVersion = settings.getValue("settings:rpi_cam_version", 1);
    stillCompression = settings.getValue("settings:still_compression", 100);

    // camera
    if (videoColor) {
        gray.allocate(width, height, OF_IMAGE_COLOR);
    } else {
        gray.allocate(width, height, OF_IMAGE_GRAYSCALE);        
    }
    
    //cam.setup(width, height, camFramerate, videoColor); // color/gray;
	grabberSetup(camId, camFramerate, width, height);

    camRotation = settings.getValue("settings:cam_rotation", 0); 
    camSharpness = settings.getValue("settings:sharpness", 0); 
    camContrast = settings.getValue("settings:contrast", 0); 
    camBrightness = settings.getValue("settings:brightness", 50); 
    camIso = settings.getValue("settings:iso", 300); 
    camExposureMode = settings.getValue("settings:exposure_mode", 0); 
    camExposureCompensation = settings.getValue("settings:exposure_compensation", 0); 
    camShutterSpeed = settings.getValue("settings:shutter_speed", 0);

    /*
    cam.setRotation(camRotation);
    cam.setSharpness(camSharpness);
    cam.setContrast(camContrast);
    cam.setBrightness(camBrightness);
    cam.setISO(camIso);
    cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
    cam.setExposureCompensation(camExposureCompensation);
    cam.setShutterSpeed(camShutterSpeed);
    //cam.setFrameRate // not implemented in ofxCvPiCam 
	*/

    // ~ ~ ~   get a persistent name for this computer   ~ ~ ~
    // a randomly generated id
    uniqueId = "RPi";
    file.open(ofToDataPath("unique_id.txt"), ofFile::ReadWrite, false);
    ofBuffer buff;
    if (file) { // use existing file if it's there
        buff = file.readToBuffer();
        uniqueId = buff.getText();
    } else { // otherwise make a new one
        uniqueId += "_" + ofGetTimestampString("%y%m%d%H%M%S%i");
        ofStringReplace(uniqueId, "\n", "");
        ofStringReplace(uniqueId, "\r", "");
        buff.set(uniqueId.c_str(), uniqueId.size());
        ofBufferToFile("unique_id.txt", buff);
    }
   
    // the actual RPi hostname
    ofSystem("cp /etc/hostname " + ofToDataPath("DocumentRoot/js/"));
    hostName = ofSystem("cat /etc/hostname");
    hostName.pop_back(); // last char is \n
    
    fbo.allocate(width, height, GL_RGBA);
    pixels.allocate(width, height, OF_IMAGE_COLOR);
        
    thresholdValue = settings.getValue("settings:threshold", 127); 
    contourThreshold = 2.0;
    contourMinAreaRadius = 1.0;
    contourMaxAreaRadius = 250.0;   
    contourFinder.setMinAreaRadius(contourMinAreaRadius);
    contourFinder.setMaxAreaRadius(contourMaxAreaRadius);
    //contourFinder.setInvert(true); // find black instead of white
    trackingColorMode = TRACK_COLOR_RGB;

    if (sendMjpeg) {
        // * stream video *
        // https://github.com/bakercp/ofxHTTP/blob/master/libs/ofxHTTP/include/ofx/HTTP/IPVideoRoute.h
        // https://github.com/bakercp/ofxHTTP/blob/master/libs/ofxHTTP/src/IPVideoRoute.cpp
        streamSettings.setPort(streamPort);
        streamSettings.ipVideoRouteSettings.setMaxClientConnections(settings.getValue("settings:max_stream_connections", 5)); // default 5
        streamSettings.ipVideoRouteSettings.setMaxClientBitRate(settings.getValue("settings:max_stream_bitrate", 512)); // default 1024
        streamSettings.ipVideoRouteSettings.setMaxClientFrameRate(settings.getValue("settings:max_stream_framerate", 30)); // default 30
        streamSettings.ipVideoRouteSettings.setMaxClientQueueSize(settings.getValue("settings:max_stream_queue", 10)); // default 10
        streamSettings.ipVideoRouteSettings.setMaxStreamWidth(width); // default 1920
        streamSettings.ipVideoRouteSettings.setMaxStreamHeight(height); // default 1080
        streamSettings.fileSystemRouteSettings.setDefaultIndex("live_view.html");
        streamServer.setup(streamSettings);
        streamServer.start();
        cout << "Using MJPEG stream." << endl;
    }

    if (sendHttp) {
        // * post form *
        // https://bakercp.github.io/ofxHTTP/classofx_1_1_h_t_t_p_1_1_simple_post_server_settings.html
        // https://github.com/bakercp/ofxHTTP/blob/master/libs/ofxHTTP/src/PostRoute.cpp
        postSettings.setPort(postPort);
        postSettings.postRouteSettings.setUploadRedirect("result.html");
        postServer.setup(postSettings);
        postServer.postRoute().registerPostEvents(this);
        postServer.start();
        cout << "Using HTTP server." << endl;
    }
        
    if (sendWs) {
        // * websockets *
        // https://github.com/bakercp/ofxHTTP/blob/master/libs/ofxHTTP/include/ofx/HTTP/WebSocketConnection.h
        // https://github.com/bakercp/ofxHTTP/blob/master/libs/ofxHTTP/src/WebSocketConnection.cpp
        // events: connect, open, close, idle, message, broadcast
        wsSettings.setPort(wsPort);
        wsServer.setup(wsSettings);
        wsServer.webSocketRoute().registerWebSocketEvents(this);
        wsServer.start();
        cout << "Using websockets." << endl;
    }
    
    if (sendOsc) {
        sender.setup(oscHost, oscPort);
        cout << "Using OSC." << endl;
    }
}

void ofApp::grabberSetup(int _id, int _fps, int _width, int _height) {
    //get back a list of devices.
    vector<ofVideoDevice> devices = cam.listDevices();

    for(size_t i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            //log the device
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            //log the device and note it as unavailable
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }

    cam.setDeviceID(_id);
    cam.setDesiredFrameRate(_fps);
    cam.initGrabber(_width, _height);
}

//--------------------------------------------------------------
void ofApp::update() {
    timestamp = (int) ofGetSystemTimeMillis();
    
    //frame = cam.grab();

    //if (!frame.empty()) {
        //toOf(frame, gray.getPixelsRef());

    cam.update();

    if (cam.isFrameNew()) {
		ofPixels tempPixels = cam.getPixelsRef();
		gray.setFromPixels(tempPixels);
		frame = toCv(gray);
		
        if (sendMjpeg) streamServer.send(gray.getPixels());
        
        if (syncVideo) {
            switch(syncVideoQuality) {
                case 5:
                    ofSaveImage(gray, videoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_BEST);
                    break;
                case 4:
                    ofSaveImage(gray, videoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_HIGH);
                    break;
                case 3:
                    ofSaveImage(gray, videoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_MEDIUM);
                    break;
                case 2:
                    ofSaveImage(gray, videoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_LOW);
                    break;
                case 1:
                    ofSaveImage(gray, videoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_WORST);
                    break;
            }
       	}
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(0);

    if(cam.isFrameNew()) {
		fbo.begin();
		
		if (debug) {
			if (!blobs && !contours) {
				drawMat(frame, 0, 0);
			} else if (blobs || contours) {
				drawMat(frameProcessed, 0, 0);
			}
		}

		if (syncVideo) {
			if (sendOsc) sendOscVideo();
			if (sendWs) sendWsVideo();
		} 

		if (blobs) {
			if (debug) {
				ofSetLineWidth(2);
				ofNoFill();
			}
			
			//autothreshold(frameProcessed);        
			threshold(frame, frameProcessed, thresholdValue, 255, 0);
			contourFinder.setThreshold(contourThreshold);    
			contourFinder.findContours(frameProcessed);

			int n = contourFinder.size();
			for (int i = 0; i < n; i++) {
				float circleRadius;
				glm::vec2 circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
				if (debug) {
					ofSetColor(cyanPrint);
					ofDrawCircle(circleCenter, circleRadius);
					ofDrawCircle(circleCenter, 1);
				}

				if (sendOsc) sendOscBlobs(i, circleCenter.x, circleCenter.y);
				if (sendWs) sendWsBlobs(i, circleCenter.x, circleCenter.y);
			}
		}

		if (contours) {
			if (debug) {
				ofSetLineWidth(2);
				ofNoFill();
			}

			int contourCounter = 0;
			unsigned char * pixels = gray.getPixels().getData();
			int gw = gray.getWidth();

			for (int h=0; h<255; h += int(255/contourSlices)) {
				contourFinder.setThreshold(h);
				contourFinder.findContours(frame);
				contourFinder.draw();            

				int n = contourFinder.size();
				for (int i = 0; i < n; i++) {
					ofPolyline line = contourFinder.getPolyline(i);
					vector<glm::vec3> cvPoints = line.getVertices();

					int x = int(cvPoints[0].x);
					int y = int(cvPoints[0].y);
					ofColor col = pixels[x + y * gw];
					float colorData[3]; 
					colorData[0] = col.r;
					colorData[1] = col.g;
					colorData[2] = col.b;
					char const * pColor = reinterpret_cast<char const *>(colorData);
					std::string colorString(pColor, pColor + sizeof colorData);
					contourColorBuffer.set(colorString); 

					float pointsData[cvPoints.size() * 2]; 
					for (int j=0; j<cvPoints.size(); j++) {
						int index = j * 2;
						pointsData[index] = cvPoints[j].x;
						pointsData[index+1] = cvPoints[j].y;
					}
					char const * pPoints = reinterpret_cast<char const *>(pointsData);
					std::string pointsString(pPoints, pPoints + sizeof pointsData);
					contourPointsBuffer.set(pointsString); 

					if (sendOsc) sendOscContours(contourCounter);
					if (sendWs) sendWsContours(contourCounter);
					contourCounter++;
				}        
			}
		}
		   
		if (brightestPixel) {
			// this mostly useful as a performance baseline
			// https://openframeworks.cc/ofBook/chapters/image_processing_computer_vision.html
			float maxBrightness = 0; 
			float maxBrightnessX = 0; 
			float maxBrightnessY = 0;
			int skip = 2;

			for (int y=0; y<height - skip; y += skip) {
				for (int x=0; x<width - skip; x += skip) {
					ofColor colorAtXY = gray.getColor(x, y);
					float brightnessOfColorAtXY = colorAtXY.getBrightness();
					if (brightnessOfColorAtXY > maxBrightness && brightnessOfColorAtXY > thresholdValue) {
						maxBrightness = brightnessOfColorAtXY;
						maxBrightnessX = x;
						maxBrightnessY = y;
					}
				}
			}

			if (debug) {
				ofNoFill();
				glm::vec2 circleCenter = glm::vec2(maxBrightnessX, maxBrightnessY);
				ofDrawCircle(circleCenter, 40);
			}

			if (sendOsc) sendOscPixel(maxBrightnessX, maxBrightnessY);
			if (sendWs) sendWsPixel(maxBrightnessX, maxBrightnessY);
		}
		
		fbo.end();
    }

    if (debug) {
		fbo.draw(0,0);
		
        stringstream info;
        info << cam.getWidth() << "x" << cam.getHeight() << " @ "<< ofGetFrameRate() <<"fps"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}

// ~ ~ ~ POST ~ ~ ~
void ofApp::onHTTPPostEvent(ofxHTTP::PostEventArgs& args) {
    ofLogNotice("ofApp::onHTTPPostEvent") << "Data: " << args.getBuffer().getText();

    takePhoto();
}


void ofApp::onHTTPFormEvent(ofxHTTP::PostFormEventArgs& args) {
    ofLogNotice("ofApp::onHTTPFormEvent") << "";
    ofxHTTP::HTTPUtils::dumpNameValueCollection(args.getForm(), ofGetLogLevel());
    
    takePhoto();
}


void ofApp::onHTTPUploadEvent(ofxHTTP::PostUploadEventArgs& args) {
    std::string stateString = "";

    switch (args.getState()) {
        case ofxHTTP::PostUploadEventArgs::UPLOAD_STARTING:
            stateString = "STARTING";
            break;
        case ofxHTTP::PostUploadEventArgs::UPLOAD_PROGRESS:
            stateString = "PROGRESS";
            break;
        case ofxHTTP::PostUploadEventArgs::UPLOAD_FINISHED:
            stateString = "FINISHED";
            break;
    }

    ofLogNotice("ofApp::onHTTPUploadEvent") << "";
    ofLogNotice("ofApp::onHTTPUploadEvent") << "         state: " << stateString;
    ofLogNotice("ofApp::onHTTPUploadEvent") << " formFieldName: " << args.getFormFieldName();
    ofLogNotice("ofApp::onHTTPUploadEvent") << "orig. filename: " << args.getOriginalFilename();
    ofLogNotice("ofApp::onHTTPUploadEvent") <<  "     filename: " << args.getFilename();
    ofLogNotice("ofApp::onHTTPUploadEvent") <<  "     fileType: " << args.getFileType().toString();
    ofLogNotice("ofApp::onHTTPUploadEvent") << "# bytes xfer'd: " << args.getNumBytesTransferred();
}

// ~ ~ ~ WEBSOCKETS ~ ~ ~
void ofApp::onWebSocketOpenEvent(ofxHTTP::WebSocketEventArgs& evt) {
    cout << "Websocket connection opened." << endl;// << evt.getConnectionRef().getClientAddress().toString() << endl;
}

void ofApp::onWebSocketCloseEvent(ofxHTTP::WebSocketCloseEventArgs& evt) {
    cout << "Websocket connection closed." << endl; //<< evt.getConnectionRef().getClientAddress().toString() << endl;
}

void ofApp::onWebSocketFrameReceivedEvent(ofxHTTP::WebSocketFrameEventArgs& evt) {
    cout << "Websocket frame was received:" << endl; // << evt.getConnectionRef().getClientAddress().toString() << endl;
    string msg = evt.frame().getText();
    cout <<  msg << endl;

    if (msg == "take_photo") {
        takePhoto();
    } else if (msg == "stream_photo") {
        streamPhoto();
    }
}

void ofApp::onWebSocketFrameSentEvent(ofxHTTP::WebSocketFrameEventArgs& evt) {
    cout << "Websocket frame was sent." << endl;
}


void ofApp::onWebSocketErrorEvent(ofxHTTP::WebSocketErrorEventArgs& evt) {
    cout << "Websocket Error." << endl; //<< evt.getConnectionRef().getClientAddress().toString() << endl;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
void ofApp::createResultHtml(string fileName) {
    string photoIndexFileName = "DocumentRoot/result.html";
    ofBuffer buff;
    ofFile photoIndexFile;
    photoIndexFile.open(ofToDataPath(photoIndexFileName), ofFile::ReadWrite, false);

    string photoIndex = "<!DOCTYPE html>\n";
    
    if (fileName == "none") { // use existing file if it's there
        photoIndex += "<html><head><meta http-equiv=\"refresh\" content=\"0\"></head><body>\n";
        photoIndex += "WAIT\n";
    } else { // otherwise make a new one
        photoIndex += "<html><head></head><body>\n";
        
        string lastFile = ofFilePath::getFileName(fileName);

        //lastPhotoTakenName = ofFilePath::getFileName(fileName);
        lastPhotoTakenName = hostName + "_" + lastFile;
        ofSystem("mv " + ofToDataPath("DocumentRoot/photos/" + lastFile) + " " + ofToDataPath("DocumentRoot/photos/" + lastPhotoTakenName));
        
        photoIndex += "<a href=\"photos/" + lastPhotoTakenName + "\">" + lastPhotoTakenName + "</a>\n";
    }

    photoIndex += "</body></html>\n";

    buff.set(photoIndex.c_str(), photoIndex.size());
    ofBufferToFile(photoIndexFileName, buff);
}

void ofApp::takePhoto() {
    ofSaveImage(gray, photoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_BEST);
    string fileName = "photo_" + ofToString(timestamp) + ".jpg";
    ofBufferToFile(ofToDataPath("DocumentRoot/photos/") + fileName, photoBuffer);
    createResultHtml(fileName);

    //string msg = "{\"unique_id\":" + uniqueId + ",\"hostname\":" + hostName + ",\"photo\":" + ofxCrypto::base64_encode(photoBuffer) + ",\"timestamp\":" + ofToString(timestamp) + "}";
    string msg = hostName + "," + lastPhotoTakenName;
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::streamPhoto() {
    ofSaveImage(gray, photoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_BEST);
    //string fileName = "photo_" + ofToString(timestamp) + ".jpg";
    //ofBufferToFile(ofToDataPath("DocumentRoot/photos/") + fileName, photoBuffer);
    //createResultHtml(fileName);

    string photo64 = ofxCrypto::base64_encode(photoBuffer);
    string msg = "{\"unique_id\":\"" + uniqueId + "\",\"hostname\":\"" + hostName + "\",\"photo\":\"" + photo64 + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

// ~ ~ ~ OSC ~ ~ ~
void ofApp::sendOscVideo() {
    ofxOscMessage m;
    m.setAddress("/video");

    m.addStringArg(hostName);    
    m.addStringArg(uniqueId);    
    m.addBlobArg(videoBuffer);
    m.addIntArg(timestamp);
    
    sender.sendMessage(m);
}

void ofApp::sendOscBlobs(int index, float x, float y) {
    ofxOscMessage m;
    m.setAddress("/blob");

    m.addStringArg(hostName);   
    m.addStringArg(uniqueId);
    m.addIntArg(index);  
    m.addFloatArg(x / (float) width);
    m.addFloatArg(y / (float) height);
    m.addIntArg(timestamp);

    sender.sendMessage(m);
}

void ofApp::sendOscContours(int index) {
    ofxOscMessage m;
    m.setAddress("/contour");
    
    m.addStringArg(hostName);
    m.addStringArg(uniqueId);
    m.addIntArg(index);
    m.addBlobArg(contourColorBuffer);
    m.addBlobArg(contourPointsBuffer);
    m.addIntArg(timestamp);

    sender.sendMessage(m);
}

void ofApp::sendOscPixel(float x, float y) {
    ofxOscMessage m;
    m.setAddress("/pixel");

    m.addStringArg(hostName);   
    m.addStringArg(uniqueId);   
    m.addFloatArg(x / (float) width);
    m.addFloatArg(y / (float) height);
    m.addIntArg(timestamp);
    
    sender.sendMessage(m);
}

// ~ ~ ~ ~ ~ 

void ofApp::sendWsVideo() { 
    string msg = "{\"unique_id\":\"" + uniqueId + "\",\"hostname\":\"" + hostName + "\",\"video\":\"" + ofxCrypto::base64_encode(videoBuffer) + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::sendWsBlobs(int index, float x, float y) {   
    float xPos = x / (float) width;
    float yPos = y / (float) height;
    
    string msg = "{\"unique_id\":\"" + uniqueId + "\",\"hostname\":\"" + hostName + "\",\"index\":\"" + ofToString(index) + "\",\"x\":\"" + ofToString(xPos) + "\",\"y\":\"" + ofToString(yPos) + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::sendWsContours(int index) {
    string msg = "{\"unique_id\":\"" + uniqueId + "\",\"hostname\":\"" + hostName + "\",\"index\":\"" + ofToString(index) + "\",\"colors\":\"" + ofxCrypto::base64_encode(contourColorBuffer) + "\",\"points\":\"" + ofxCrypto::base64_encode(contourPointsBuffer) + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::sendWsPixel(float x, float y) {   
    float xPos = x / (float) width;
    float yPos = y / (float) height;

    string msg = "{\"unique_id\":\"" + uniqueId + "\",\"hostname\":\"" + hostName + "\",\"x\":\"" + ofToString(xPos) + "\",\"y\":\"" + ofToString(yPos) + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::keyPressed(int key) {
    // in fullscreen mode, on a pc at least, the 
    // first time video settings the come up
    // they come up *under* the fullscreen window
    // use alt-tab to navigate to the settings
    // window. we are working on a fix for this...

    // Video settings no longer works in 10.7
    // You'll need to compile with the 10.6 SDK for this
    // For Xcode 4.4 and greater, see this forum post on instructions on installing the SDK
    // http://forum.openframeworks.cc/index.php?topic=10343
    if (key == 's' || key == 'S') {
        cam.videoSettings();
    }
}
