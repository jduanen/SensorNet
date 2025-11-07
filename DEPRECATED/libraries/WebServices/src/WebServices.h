/***************************************************************************
*
* Library that handles Web Server, Web Sockets, and OTA updates
* 
****************************************************************************/

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>


#define WEB_SERVICES_VERSION    "2.0"

#define DEF_PORT_NUM            80
#define WEB_SOCKET_PATH         "/ws"
#define MAX_NUM_PAGES           3

#define CALL_MEMBER_FUNC(obj, memberPtr)    ((obj).*(memberPtr))


typedef String (*wsMsgHandler)(const JsonDocument& wsMsg);

typedef struct WebPageDefStruct {
    const char *htmlPath;
    const char *scriptsPath;
    const char *stylePath;
    AwsTemplateProcessor htmlProcessor;
    wsMsgHandler msgHandler;
} WebPageDef;


template<int maxMsgSize>
class WebServices {
public:
    String libVersion = WEB_SERVICES_VERSION;

    WebServices(const String& applName, const uint16_t portNum=DEF_PORT_NUM, const String& configPath="");

    bool addPage(const WebPageDef& pageDef);
    bool addPage(const char* filePath, const char* mimeType);

    void run();

    void updateClients();

private:
    String _applName;
    bool _verbose = true;
    uint16_t _numPages = 0;

    AsyncElegantOtaClass AsyncElegantOTA;

    AsyncWebServer *_serverPtr = NULL;
    AsyncWebSocket *_socketPtr = NULL;

    DynamicJsonDocument *_wsMsgPtr;

    WebPageDef _pageDefs[MAX_NUM_PAGES];

    void _print(String str);
    void _println(String str);

    void _onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len);
    void _handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void _notifyClients(const JsonDocument& doc);
};

#include "WebServices.hpp"
