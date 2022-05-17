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

WebServices::WebServices(String applName, const uint16_t portNum) {
    _applName = applName;
    _println("webServices for " + _applName);
    _serverPtr = new AsyncWebServer(portNum);
    AsyncElegantOTA.begin(_serverPtr);
    _serverPtr->begin();
}

void WebServices::setup(String configPath) {
    setup(configPath, "", "");
}

void WebServices::setup(String configPath, String commonPagePath) {
    setup(configPath, commonPagePath, "");
}

void WebServices::setup(String configPath, String commonPagePath, String applPagePath) {
    _print("webServices::setup ");
    if (!LittleFS.begin()) {
        _println("ERROR: WebServices::setup failed to mount LittleFS");
        return;
    }
    if (configPath != "") {
        _print("configPath=" + configPath);
    }
    if (commonPagePath != "") {
        _socketPtr = new AsyncWebSocket("/ws");
        _socketPtr->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {_onEvent(server, client, type, arg, data, len);});
        _serverPtr->addHandler(_socketPtr);
        _print("commonPagePath=" + commonPagePath);
        _serverPtr->on("/common/wsStyle.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/common/wsStyle.css", "text/css");
        });
        _serverPtr->on("/common/wsScripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/common/wsScripts.js", "text/javascript");
        });

        _serverPtr->on(commonPagePath.c_str(), HTTP_GET, [&](AsyncWebServerRequest *request){
//            request->send_P(200, "text/html", index_html, _commonProcessor);
//        _serverPtr->on(rootPagePath.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){
//            request->send_P(200, "text/html", index_html, [this](String str) -> String { _commonProcessor(str); });
            request->send_P(200, "text/html", index_html);
        });
    }
    if (applPagePath != "") {
        // N.B. this assumes that the common page is always enabled when an application-specific page is enabled
        _print("applPagePath=" + applPagePath);
        _serverPtr->on("/appl/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/appl/style.css", "text/css");
        });
        _serverPtr->on("/appl/wsScripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/appl/scripts.js", "text/javascript");
        });
    }
    _println(".");
}

void WebServices::run() {
//    _println("webServices::run");
    if (_socketPtr != NULL) {
        _socketPtr->cleanupClients();
    }
}

String WebServices::_commonProcessor(const String& var) {
  if (var == "APPL_NAME") {
    return (_applName);
  } else if (var == "LIB_VERSION") {
    return (libVersion);
  } else if (var == "IP_ADDR") {
    return (WiFi.localIP().toString());
  } else if (var == "SSID") {
    return (WiFi.SSID());
  } else if (var == "RSSI") {
    return (String(WiFi.RSSI()));
  }
  return String();
}

String WebServices::_applProcessor(const String& var) {
    // N.B. This is to be overridden by application-specific code
    // returns the actual string to be used in place of the given template value
    return String();
}

void WebServices::_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      _println("WebSocket client #" + String(client->id()) + " connected from " + String(client->remoteIP().toString().c_str()));
      break;
    case WS_EVT_DISCONNECT:
      _println("WebSocket client #" + String(client->id()) + " disconnected");
      break;
    case WS_EVT_DATA:
      _handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void WebServices::_handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        DeserializationError error = deserializeJson(_wsMsg, (char *)data);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
            return;
        };
        String m;
        serializeJsonPretty(_wsMsg, m);
        _println("Msg: " + m);
        // TODO add any common actions in response to the WS message
        String msgType = String(_wsMsg["msgType"]);
        if (msgType.equals("query")) {
            // NOP
        } else if (msgType.equals("saveConf")) {
            //// FIXME add code here
        }

        // notify clients of current state via a stringified JSON object
        String msg = "{\"applName\": \"" + _applName + "\"";
        msg += ", \"libVersion\": \"" + libVersion + "\"";
        msg += ", \"ipAddr\": \"" + WiFi.localIP().toString() + "\"";
        msg += ", \"connected\": \"" + WiFi.SSID() + "\"";
        msg += ", \"RSSI\": \"" + String(WiFi.RSSI()) + "\"";
        msg += _applWSMsgHandler();
        msg += "}";
        Serial.println("MSG: " + msg); //// TMP TMP TMP
      _socketPtr->textAll(msg);
    }
}

String WebServices::_applWSMsgHandler() {
    // N.B. This is to be overridden by application-specific code
    // returns serialized JSON object to be sent in client notification message
    String msg = ", \"applName\": " + String("\"?\"");
    msg += ", \"applVersion\": " + String("\"?\"");
    return(msg);
}

        /*
        String msgType = String(wsMsg["msgType"]);
        if (msgType.equals("led")) {
            configState.ledState = !configState.ledState;
        } else if (msgType.equals("el")) {
            configState.elState = !configState.elState;
        } else {
            Serial.println("Error: unknown message type: " + msgType);
            return;
        }
        */

/*
private:
    AsyncWebServer *_serverPtr;
    AsyncWebSocket *_socketPtr;

    ConfigStorage *_csPtr;
    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;
*/
