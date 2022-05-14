/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#include "WebServices.h"


void WebServices::_print(String str) {
  if (VERBOSE) {
    Serial.print(str);
  }
}

void WebServices::_println(String str) {
  if (VERBOSE) {
    Serial.println(str);
  }
}

WebServices::WebServices(const uint16_t portNum) {
    _println("webServices::setup");
    _serverPtr = new AsyncWebServer(portNum);
    AsyncElegantOTA.begin(_serverPtr);
    _serverPtr->begin();
}

void WebServices::setup(String configPath, String rootPagePath) {
    _println("webServices::setup");
}

void WebServices::run() {
//    _println("webServices::run");
}

String WebServices::_commonProcessor(const String& var) {
    _println("webServices::_commonProcessor");
}

/*
private:
    AsyncWebServer *_serverPtr;
    AsyncWebSocket *_socketPtr;

    ConfigStorage *_csPtr;
    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;
*/
