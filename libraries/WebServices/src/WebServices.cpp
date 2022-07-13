/*
* Library that handles Web Server, Web Sockets, and OTA updates for SensorNet devices
*/

#include "WebServices.h"


void WebServices::_print(String str) {
  if (_verbose) {
    Serial.print(str);
  }
}

void WebServices::_println(String str) {
  if (_verbose) {
    Serial.println(str);
  }
}

WebServices::WebServices(const String& applName, const uint16_t portNum, const String& configPath) {
    _applName = applName;
    _print("webServices for " + _applName + ", port number: " + String(portNum));
    if (configPath == "") {
        _println("");
    } else {
        _println(", configPath=" + configPath);
        //// TODO set up config feature
    }
    _serverPtr = new AsyncWebServer(portNum);
    AsyncElegantOTA.begin(_serverPtr);
    _serverPtr->begin();
    if (!LittleFS.begin()) {
        _println("ERROR: WebServices::setup failed to mount LittleFS");
        return;
    }
}

bool WebServices::addPage(const WebPageDef& pageDef) {
    if (_numPages >= MAX_NUM_PAGES) {
        Serial.println("ERROR: too many pages");
        return(false);
    }
    if (pageDef.msgHandler == nullptr) {
        Serial.println("ERROR: page must include WS message handler");
        return(false);
    }
    if (pageDef.htmlPath == nullptr) {
        Serial.println("ERROR: page must include HTML");
        return(false);
    }

    if (_socketPtr == NULL) {
        _socketPtr = new AsyncWebSocket("/ws");
        _socketPtr->onEvent([this](AsyncWebSocket *server,
                                   AsyncWebSocketClient *client,
                                   AwsEventType type,
                                   void *arg,
                                   uint8_t *data,
                                   size_t len) {_onEvent(server, client, type, arg, data, len);});
        _serverPtr->addHandler(_socketPtr);
    }

    if (pageDef.htmlProcessor == nullptr) {
        _serverPtr->on(pageDef.htmlPath,
                       HTTP_GET,
                       [=](AsyncWebServerRequest *request) {
                            request->send(LittleFS, pageDef.htmlPath, "text/html");});
    } else {
        _serverPtr->on(pageDef.htmlPath,
                       HTTP_GET,
                       [=](AsyncWebServerRequest *request){
                            request->send(LittleFS, pageDef.htmlPath, "text/html", false, pageDef.htmlProcessor);});
    }
    if (pageDef.scriptsPath != nullptr) {
        _serverPtr->on(pageDef.stylePath,
                       HTTP_GET,
                       [=](AsyncWebServerRequest *request){
                            request->send(LittleFS, pageDef.stylePath, "text/css");});
    }
    if (pageDef.scriptsPath != nullptr) {
        _serverPtr->on(pageDef.scriptsPath,
                       HTTP_GET,
                       [=](AsyncWebServerRequest *request) {
                            request->send(LittleFS, pageDef.scriptsPath, "text/javascript");});
    }

    _pageDefs[_numPages++] = pageDef;
    return(true);
}

void WebServices::run() {
//    _println("webServices::run");
    if (_socketPtr != NULL) {
        _socketPtr->cleanupClients();
    }
}

void WebServices::updateClients() {
    if (_numPages < 1) {
        _println("WARNING: no pages registered, skipping updateClients");
        return;
    }
    StaticJsonDocument<32> doc;
    doc["msgType"] = "update";
    _notifyClients(doc);
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
        _println("Rx Msg: " + m);  //// TMP TMP TMP
        _notifyClients(_wsMsg);
    }
}

void WebServices::_notifyClients(const JsonDocument& doc) {
    String msg = "{\"applName\": \"" + _applName + "\"";
    for (int i = 0; (i < _numPages); i++) {
        msg += _pageDefs[i].msgHandler(doc);
    }
    msg += "}";
    Serial.println("Notify Msg: " + msg);  //// TMP TMP TMP
    _socketPtr->textAll(msg);
}
