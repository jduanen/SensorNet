/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#ifndef WEB_SERVICES_H
#define WEB_SERVICES_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <AsyncElegantOTA.h>


#define LIB_VERSION             "1.0"


const char _indexHtml[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>Hello World!</html>)rawliteral";


class WebServices {
public:
    WebServices(const uint16_t portNum);

    void webServicesRun();

private:
    AsyncWebServer *_webServerPtr;
    void _webServicesSetup();

    static String _processor(const String &var);
};

#endif /*WEB_SERVICES_H*/
