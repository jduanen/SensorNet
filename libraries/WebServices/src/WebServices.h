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

#define VERBOSE                 1
#define LIB_VERSION             "1.0"
#define DEF_CONFIG_PATH         "/config.json"
#define DEF_PORT_NUM            80
#define WEB_SOCKET_PATH         "/ws"
#define WS_JSON_DOC_SIZE        256

#define COMMON_STYLE_PATH       "/common/wsStyle.css"
#define COMMON_SCRIPT_PATH      "/common/wsScripts.js"

#define CALL_MEMBER_FUNC(obj, memberPtr)    ((obj).*(memberPtr))


////typedef String (htmlProcessor)(const String& var);  #### Use AwsTemplateProcessor instead
typedef String (*wsMsgHandler)(StaticJsonDocument<WS_JSON_DOC_SIZE>& wsMsg);


class WebServices {
public:
    String libVersion = LIB_VERSION;

    WebServices(const String& applName, const uint16_t portNum=DEF_PORT_NUM, const String& configPath="");

    void addPage(const String& htmlPath="", const String& stylePath="", const String& scriptsPath="", AwsTemplateProcessor processor=nullptr);

    void run();

    String commonMsgHandler(StaticJsonDocument<WS_JSON_DOC_SIZE> wsMsg);

private:
    String _applName;

    AsyncElegantOtaClass AsyncElegantOTA;

    AsyncWebServer *_serverPtr = NULL;
    AsyncWebSocket *_socketPtr = NULL;

    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;

//    wsMsgHandler *_msgHandlers;
    wsMsgHandler _msgHandlers[3];

    void _print(String str);
    void _println(String str);

    void _notifyClients();
    void _onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len);
    void _handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
//    static String _commonProcessor(const String& var);
};

#endif /*WEB_SERVICES_H*/
