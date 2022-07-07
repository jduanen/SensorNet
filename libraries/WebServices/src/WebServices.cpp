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

/*
void WebServices::addPage(const String& htmlPath, const String& stylePath, const String& scriptsPath, AwsTemplateProcessor processor) {
    if ((htmlPath == "") && (stylePath == "") && (scriptsPath == "")) {
        Serial.println("ERROR: all paths are empty");
        return;
    }
    _print("addPage: " + htmlPath + ", stylePage: " + stylePath + ", scriptsPage: " + scriptsPath);
    if ((htmlPath != "") && !LittleFS.exists(htmlPath)) {
        Serial.println("ERROR: invalid HTML page path -- " + htmlPath);
        return;
    }
    if ((stylePath != "") && !LittleFS.exists(stylePath)) {
        Serial.println("ERROR: invalid style page path -- " + stylePath);
        return;
    }
    if ((scriptsPath != "") && !LittleFS.exists(scriptsPath)) {
        Serial.println("ERROR: invalid scripts page path -- " + scriptsPath);
        return;
    }

    if (_socketPtr == NULL) {
        _socketPtr = new AsyncWebSocket("/ws");
        _socketPtr->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {_onEvent(server, client, type, arg, data, len);});
        _serverPtr->addHandler(_socketPtr);
    }

    // "/index.html"
    if (htmlPath != "") {
        //// FIXME figure out how to make pre-processing work
        if (processor == nullptr) {
            _serverPtr->on(htmlPath.c_str(), HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, htmlPath.c_str(), "text/html");
            });
        } else {
            _serverPtr->on(htmlPath.c_str(), HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, htmlPath.c_str(), "text/html", false, processor);
            });
        }
        _msgHandlers[0] = &WebServices::commonMsgHandler; //// TMP TMP TMP
    }
    // "/common/wsStyle.css"
    if (stylePath != "") {
        _serverPtr->on(stylePath.c_str(), HTTP_GET, [=](AsyncWebServerRequest *request){
            request->send(LittleFS, stylePath.c_str(), "text/css");
        });
    }
    // "/common/wsScripts.js"
    if (scriptsPath != "") {
        //// FIXME figure out how to make pre-processing work
        if (processor == nullptr) {
            _serverPtr->on(scriptsPath.c_str(), HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, scriptsPath.c_str(), "text/javascript");
            });
        } else {
            _serverPtr->on(scriptsPath.c_str(), HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, scriptsPath.c_str(), "text/javascript", false, processor);
            });
        }
    }
//
//        _serverPtr->on(htmlPath.c_str(), HTTP_GET, [&](AsyncWebServerRequest *request){
//            request->send_P(200, "text/html", index_html);
//            request->send_P(200, "text/html", index_html, _commonProcessor);
//        _serverPtr->on(rootPagePath.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){
//            request->send_P(200, "text/html", index_html, [this](String str) -> String { _commonProcessor(str); });
//        });
//    }
//
}
*/

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

        // notify clients of current state via a stringified JSON object
        String msg = "{\"applName\": \"" + _applName + "\"";
        Serial.println("AAAAAAAA");
        for (int i = 0; (i < MAX_NUM_PAGES); i++) {
            Serial.println("BBBBBBBBB");
            msg += _pageDefs[i].msgHandler(_wsMsg);
        }
        msg += "}";
        Serial.println("MSG: " + msg); //// TMP TMP TMP
        _socketPtr->textAll(msg);
    }
}
