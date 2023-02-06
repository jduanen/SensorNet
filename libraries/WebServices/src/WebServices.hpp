/***************************************************************************
 *
 * Library that handles Web Server, Web Sockets, and OTA updates
 * 
 ***************************************************************************/

template<int maxMsgSize>
void WebServices<maxMsgSize>::_print(String str) {
  if (_verbose) {
    Serial.print(str);
  }
}

template<int maxMsgSize>
void WebServices<maxMsgSize>::_println(String str) {
  if (_verbose) {
    Serial.println(str);
  }
}

template<int maxMsgSize>
WebServices<maxMsgSize>::WebServices(const String& applName, const uint16_t portNum, const String& configPath) {
    _wsMsgPtr = new DynamicJsonDocument(maxMsgSize);
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
        _println("ERROR: WebServices<maxMsgSize>::setup failed to mount LittleFS");
        return;
    }
}

template<int maxMsgSize>
bool WebServices<maxMsgSize>::addPage(const WebPageDef& pageDef) {
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

template<int maxMsgSize>
bool WebServices<maxMsgSize>::addPage(const char* filePath, const char* mimeType) {
    if ((filePath == nullptr) || (mimeType == nullptr)) {
        Serial.println("ERROR: page must include path to file and mime type strings");
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
    _serverPtr->on(filePath,
                   HTTP_GET,
                   [=](AsyncWebServerRequest *request) {
                        request->send(LittleFS, filePath, mimeType);});
	return(true);
}

template<int maxMsgSize>
void WebServices<maxMsgSize>::run() {
//    _println("WebServices<maxMsgSize>::run");
    if (_socketPtr != NULL) {
        _socketPtr->cleanupClients();
    }
}

template<int maxMsgSize>
void WebServices<maxMsgSize>::updateClients() {
    if (_numPages < 1) {
        _println("WARNING: no pages registered, skipping updateClients");
        return;
    }
    StaticJsonDocument<32> doc;
    doc["msgType"] = "update";
    _notifyClients(doc);
}

template<int maxMsgSize>
void WebServices<maxMsgSize>::_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
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

template<int maxMsgSize>
void WebServices<maxMsgSize>::_handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        DeserializationError error = deserializeJson(*_wsMsgPtr, (char *)data);
        if (error) {
            Serial.print("_handleWebSocketMessage: deserializeJson() failed: ");
            Serial.println(error.f_str());
            return;
        };
        //// TMP TMP TMP
        if (false) {
            String m;
            serializeJsonPretty(*_wsMsgPtr, m);
            Serial.println("Rx Msg: " + m);
        }
        _notifyClients(*_wsMsgPtr);
    }
}

template<int maxMsgSize>
void WebServices<maxMsgSize>::_notifyClients(const JsonDocument& doc) {
    String msg = "{\"applName\": \"" + _applName + "\"";
    for (int i = 0; (i < _numPages); i++) {
        msg += _pageDefs[i].msgHandler(doc);
    }
    msg += "}";
    //// TMP TMP TMP
    if (false) {
        Serial.println("Notify Msg: " + msg);
    }
    _socketPtr->textAll(msg);
}
