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
      <title>%APPL_NAME% Web Server</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <style>
        html {
          font-family: Arial, Helvetica, sans-serif;
          text-align: center;
        }
        h1 {
          font-size: 1.9rem;
          color: white;
        }
        h2{
          margin: 5px;
          font-size: 1.7rem;
          font-weight: bold;
          color: #143642;
        }
        h3{
          margin-top: 10px;
          font-size: 1.5rem;
          font-weight: bold;
          color: #143642;
        }
        .topnav {
          overflow: hidden;
          background-color: #143642;
        }
        body {
          margin: 0;
        }
        .content {
          padding: 30px;
          max-width: 600px;
          margin: 0 auto;
        }
        .card {
          background-color: #F8F7F9;;
          box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
          padding-top:10px;
          padding-bottom:20px;
        }
        .staticState {
          margin: 5px;
          font-size: 1.2rem;
          color:black;
          font-weight: bold;
        }
        .vertical-center {
          margin-top: 0px;
          margin-bottom: 25px;
          line-height: 2em;
          font-size: 1.2rem;
          color:#8c8c8c;
          font-weight: bold;
        }
        .green-button {
          background-color: #4CAF50;
          border: 10px;
          color: white;
          padding: 5px 8px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 1.2rem;
        }
      </style>
    </head>
    <body>
      <div class="topnav">
        <h1>%APPL_NAME% Web Server</h1>
      </div>
      <div class="content">
        <div class="card">
          <h2>Information</h2>
            <h3>Common</h3>
              <p class="staticState">IP Address: <span style="color:blue" id="ipAddr">%IP_ADDR%</span></p>
              <p class="staticState">SSID: <span style="color:blue" id="connection">%SSID%</span></p>
              <p class="staticState">RSSI: <span style="color:blue" id="rssi">%RSSI%</span></p>
            <h3>Application-Specific</h3>

        </div>
        <br>
        <div class="card">
          <h2>Controls</h2>
            <h3>Common</h3>
              <div class="vertical-center" style="line-height: 1.5em;">
                <p>Update Firmware: <a href="update">Update</a></p>
                <p>
                  SSID: <input type="text" id="ssid">
                  <br>
                  Password: <input type="password" id="password">
                </p>
              </div>
            <h3>Application-Specific</h3>

          <div class="vertical-center">
            <p><button class="green-button" id="save" onclick="saveConfiguration()">Save Configuration</button></p>
          </div>
        </div>
      </div>
      <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        window.addEventListener('load', onLoad);
        function initWebSocket() {
          console.log('Trying to open a WebSocket connection...');
          websocket = new WebSocket(gateway);
          websocket.onopen    = onOpen;
          websocket.onclose   = onClose;
          websocket.onmessage = onMessage;
        }
        function onOpen(event) {
          console.log('Connection opened');
          initView();
        }
        function onClose(event) {
          console.log('Connection closed');
          setTimeout(initWebSocket, 2000);
        }
        function onLoad(event) {
          initWebSocket();
        }
        function initView() {
          var jsonMsg = JSON.stringify({"msgType": "query"});
          websocket.send(jsonMsg);
          console.log("FIXME: initView");
        }
        function onMessage(event) {
          var state;
          var elem;
          const msgObj = JSON.parse(event.data);
          console.log('msgObj: ', msgObj);
        }
        function saveConfiguration() {
          var ssid = document.getElementById("ssid").value;
          var passwd = document.getElementById("password").value;
          var jsonMsg = JSON.stringify({"msgType": "saveConf", "ssid": ssid, "passwd": passwd});
          console.log("Save configuration: " + jsonMsg);
          websocket.send(jsonMsg);
        }
      </script>
    </body>
  </html>)rawliteral";


class WebServices {
public:
    String libVersion = LIB_VERSION;

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
