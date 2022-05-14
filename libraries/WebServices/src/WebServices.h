/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#ifndef WEB_SERVICES_H
#define WEB_SERVICES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#define CS_USE_LITTLEFS     true
#include <ConfigStorage.h>

#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>


#define VERBOSE                 1
#define LIB_VERSION             "1.0"
#define DEF_CONFIG_PATH         "/config.json"
#define WEB_SOCKET_PATH         "/ws"
#define WS_JSON_DOC_SIZE        256


typedef String (htmlProcessor)(const String& var);


class WebServices {
public:
    WebServices(const uint16_t portNum);

    void setup(String configPath, String rootPagePath);

    void run();

private:
    AsyncElegantOtaClass AsyncElegantOTA;

    AsyncWebServer *_serverPtr;
    AsyncWebSocket *_socketPtr;

    ConfigStorage *_csPtr;
    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;

    void _print(String str);
    void _println(String str);

    String _commonProcessor(const String& var);
};


#endif /*WEB_SERVICES_H*/
