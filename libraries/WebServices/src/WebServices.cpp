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
    _print("webServices::setup ");
    if (configPath != "") {
        _print("configPath=" + configPath);
    }
    if (rootPagePath != "") {
        _print("rootPagePath=" + rootPagePath);
        _socketPtr = new AsyncWebSocket("/ws");
        _socketPtr->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {_onEvent(server, client, type, arg, data, len);});
        _serverPtr->addHandler(_socketPtr);
        _serverPtr->on(rootPagePath.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html, [this](String str) -> String { _commonProcessor(str); });
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

void WebServices::_notifyClients() {
  String msg = "{";
  msg += "\"foo\": " + String(1);
  msg += "}";
  _socketPtr->textAll(msg);
}

String WebServices::_commonProcessor(const String& var) {
    _println("webServices::_commonProcessor");
}

void WebServices::_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      printf("WebSocket client #%u disconnected\n", client->id());
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
        Serial.println("Msg: ");
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
        _notifyClients();
    }
}

/*
private:
    AsyncWebServer *_serverPtr;
    AsyncWebSocket *_socketPtr;

    ConfigStorage *_csPtr;
    StaticJsonDocument<WS_JSON_DOC_SIZE> _wsMsg;
*/
