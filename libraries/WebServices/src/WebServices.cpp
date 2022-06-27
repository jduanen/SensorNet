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

WebServices::WebServices(String applName) {
    _setup(applName, DEF_PORT_NUM, "");
}

WebServices::WebServices(String applName, const uint16_t portNum) {
    _setup(applName, portNum, "");
}

WebServices::WebServices(String applName, const uint16_t portNum, String configPath) {
    _setup(applName, portNum, configPath);
}

void WebServices::_setup(String applName, const uint16_t portNum, String configPath) {
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

void WebServices::addPage(String htmlPath, String stylePath, String scriptsPath, AwsTemplateProcessor processor) {
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
    char pathBuffer[MAX_PATH_LENGTH];
    if (htmlPath != "") {
        htmlPath.toCharArray(pathBuffer, MAX_PATH_LENGTH);
        //// FIXME figure out how to make pre-processing work
        if (processor == nullptr) {
            _serverPtr->on(pathBuffer, HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, pathBuffer, "text/html");
            });
        } else {
            _serverPtr->on(pathBuffer, HTTP_GET, [=](AsyncWebServerRequest *request){
                request->send(LittleFS, pathBuffer, "text/html", false, processor);
            });
        }
    }
    // "/common/wsStyle.css"
    if (stylePath != "") {
        stylePath.toCharArray(pathBuffer, MAX_PATH_LENGTH);
        _serverPtr->on(pathBuffer, HTTP_GET, [=](AsyncWebServerRequest *request){
            request->send(LittleFS, pathBuffer, "text/css");
        });
    }
    // "/common/wsScripts.js"
    if (scriptsPath != "") {
        scriptsPath.toCharArray(pathBuffer, MAX_PATH_LENGTH);
        //// FIXME figure out how to make pre-processing work
        _serverPtr->on(pathBuffer, HTTP_GET, [=](AsyncWebServerRequest *request){
            request->send(LittleFS, pathBuffer, "text/javascript");
        });
    }
/*
        _serverPtr->on(htmlPath.c_str(), HTTP_GET, [&](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html);
//            request->send_P(200, "text/html", index_html, _commonProcessor);
//        _serverPtr->on(rootPagePath.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){
//            request->send_P(200, "text/html", index_html, [this](String str) -> String { _commonProcessor(str); });
        });
    }
*/
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

/*
//// FIXME figure out how to make processing work and then add to addPage()
static String WebServices::_commonProcessor(const String& var) {
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
*/

/*
String WebServices::_applProcessor(const String& var) {
    // N.B. This is to be overridden by application-specific code
    // returns the actual string to be used in place of the given template value
    return String();
}
*/

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
        for (wsMsgHandler hdlr : _msgHandlers) {
            msg += hdlr(_wsMsg);
        }
        msg += "}";
        Serial.println("MSG: " + msg); //// TMP TMP TMP
        _socketPtr->textAll(msg);
    }
}

String WebServices::commonMsgHandler(StaticJsonDocument<WS_JSON_DOC_SIZE> wsMsg) {
    String msgType = String(wsMsg["msgType"]);
    if (msgType.equals("query")) {
        // NOP
    } else if (msgType.equals("saveConf")) {
        //// FIXME add code here
    }

    String msg = ", \"libVersion\": \"" + libVersion + "\"";
    msg += ", \"ipAddr\": \"" + WiFi.localIP().toString() + "\"";
    msg += ", \"connected\": \"" + WiFi.SSID() + "\"";
    msg += ", \"RSSI\": \"" + String(WiFi.RSSI()) + "\"";
    return(msg);
}

/*
String WebServices::_applWSMsgHandler() {
    // N.B. This is to be overridden by application-specific code
    // returns serialized JSON object to be sent in client notification message
    String msg = ", \"applName\": " + String("\"?\"");
    msg += ", \"applVersion\": " + String("\"?\"");
    return(msg);
}
*/

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

/*
const rot13 = str => str.split('')
    .map(char => String.fromCharCode(char.charCodeAt(0) + (char.toLowerCase() < 'n' ? 13 : -13)))
    .join('');
*/

