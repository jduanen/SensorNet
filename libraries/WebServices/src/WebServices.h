/***************************************************************************
*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*
****************************************************************************/

#ifndef WEB_SERVICES_H
#define WEB_SERVICES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

#define DEF_CONFIG_PATH         "/config.json"
#define DEF_PORT_NUM            80
#define WEB_SOCKET_PATH         "/ws"
#define WS_JSON_DOC_SIZE        256
#define MAX_NUM_PAGES           3

#define COMMON_HTML_PATH        "/index.html"
#define COMMON_STYLE_PATH       "/common/wsStyle.css"
#define COMMON_SCRIPTS_PATH     "/common/wsScripts.js"

#define CALL_MEMBER_FUNC(obj, memberPtr)    ((obj).*(memberPtr))


typedef String (*wsMsgHandler)(const JsonDocument& wsMsg);

typedef struct WebPageDefStruct {
    const char *htmlPath;
    const char *scriptsPath;
    const char *stylePath;
    AwsTemplateProcessor htmlProcessor;
    wsMsgHandler msgHandler;
} WebPageDef;


class WebServices {
public:
    String libVersion = "1.1";

    WebServices(const String& applName, const uint16_t portNum=DEF_PORT_NUM, const String& configPath="");

    bool addPage(const WebPageDef& pageDef);

    void run();

    void updateClients();

private:
    String _applName;
    bool _verbose = true;
    uint16_t _numPages = 0;

    AsyncElegantOtaClass AsyncElegantOTA;

    AsyncWebServer *_serverPtr = NULL;
    AsyncWebSocket *_socketPtr = NULL;

    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;

    WebPageDef _pageDefs[MAX_NUM_PAGES];

    void _print(String str);
    void _println(String str);

    void _onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len);
    void _handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void _notifyClients(const JsonDocument& doc);
};

#endif /*WEB_SERVICES_H*/
