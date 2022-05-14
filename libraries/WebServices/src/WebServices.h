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

const char index_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
      <title>PrcDisplay Web Server</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
    </head>
    <body>
      <div class="topnav">
        <h1>PrcDisplay Web Server</h1>
      </div>
    </body>
    </html>)rawliteral";


class WebServices {
public:
    WebServices(const uint16_t portNum);

    void setup(String configPath, String rootPagePath);

    void run();

private:
    AsyncElegantOtaClass AsyncElegantOTA;

    AsyncWebServer *_serverPtr = NULL;
    AsyncWebSocket *_socketPtr = NULL;

    ConfigStorage *_csPtr;
    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;

    void _print(String str);
    void _println(String str);

    void _notifyClients();
    String _commonProcessor(const String& var);

    void _onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len);
    void _handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
};

#endif /*WEB_SERVICES_H*/
