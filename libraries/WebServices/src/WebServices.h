/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#ifndef WEB_SERVICES_H
#define WEB_SERVICES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
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
      <title>%APPL_NAME% Web Server</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <link rel="stylesheet" type="text/css" href="wsStyle.css">
    </head>
    <body>
      <div class="topnav">
        <h1>%APPL_NAME% Web Server</h1>
      </div>
      <div class="content">
        <div class="card">
          <h2>Common Information</h2>
          <div class="vertical-center" style="line-height: 1.5em;">
            <p class="staticState">IP Address: <span style="color:blue" id="ipAddr">%IP_ADDR%</span></p>
            <p class="staticState">SSID: <span style="color:blue" id="connection">%SSID%</span></p>
            <p class="staticState">RSSI: <span style="color:blue" id="rssi">%RSSI%</span></p>
          </div>
        </div>
        <br>
        <div class="card">
          <h2>Common Controls</h2>
          <div class="vertical-center" style="line-height: 1.5em;">
            <p>Update Firmware: <a href="update">Update</a></p>
            <p>
              SSID: <input type="text" id="ssid">
              <br>
              Password: <input type="password" id="password">
            </p>
          </div>
          <div class="vertical-center">
            <p><button class="green-button" id="save" onclick="saveConfiguration()">Save Configuration</button></p>
          </div>
        </div>
        <nav>
        <a href="/app">Application-Specific</a> |
        <a href="/update">Update Firmware</a>
        </nav>
      </div>
      <script src="wsScripts.js"></script>
    </body>
  </html>)rawliteral";


class WebServices {
public:
    String libVersion = LIB_VERSION;

    WebServices(const uint16_t portNum);

    void setup(String configPath);
    void setup(String configPath, String commonPagePath);
    void setup(String configPath, String commonPagePath, String applPagePath);

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
