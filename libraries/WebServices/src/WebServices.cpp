/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#include "WebServices.h"


WebServices::WebServices(const uint16_t portNum) {
    // Setup web server interface on the given port number
    Serial.println("WebServices");
    _webServerPtr = new AsyncWebServer(portNum);
    _webServicesSetup();
}

void WebServices::_webServicesSetup() {
    Serial.println("webServicesSetup");
    _webServerPtr->on("/",
                  HTTP_GET,
                  [](AsyncWebServerRequest *request){
                    request->send_P(200, "text/html", _indexHtml, _processor);});
//    AsyncElegantOTA.begin(_webServerPtr);
    _webServerPtr->begin();
}

// Code to be run in loop() to maintain the websockets interface
void WebServices::webServicesRun() {
    Serial.println("webServicesRun");
}

String WebServices::_processor(const String &var) {
    Serial.println("_processor: " + var);
    return String();
}
