/***************************************************************************
 *
 * SensorNet WebServices library test
 *
 * Notes:
 *   - ?
 *
 ***************************************************************************/

#include "wifi.h"
#include "WiFiUtilities.h"
#include "WebServices.h"
#include "ConfigService.h"


#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "1.0.1"
#define WEB_SERVER_PORT     80
#define WIFI_AP_SSID        "WebServices"

//#define USER_NAME           "name"
//#define PASSWD              "passwd"

#define CONFIG_PATH         "/config.json"

#define APPL_PAGE_PATH      "/appl"

#define FW_UPDATE           (1 << 0)    // test OTA firmware update feature
#define COMMON_GUI          (1 << 1)    // test common GUI
#define APPL_GUI            (1 << 2)    // test application-specific GUI


typedef struct {
  String ssid;
  String passwd;
} ConfigState;

ConfigState configState = {
  String(WLAN_SSID),
  String(rot47(WLAN_PASS)),
};

WebServices webSvcs(APPL_NAME, WEB_SERVER_PORT);

////int testMode = FW_UPDATE;
int testMode = FW_UPDATE | COMMON_GUI;
////int testMode = FW_UPDATE | COMMON_GUI | APPL_GUI;

int loopCnt = 0;

/*
String commonHandlerWrapper(StaticJsonDocument<WS_JSON_DOC_SIZE> wsMsg) {
    return(webSvcs.commonMsgHandler(wsMsg));
}
*/

String indexPageProcessor(const String& var) {
    Serial.println(var);
    if (var == "APPL_NAME") {
        return (String(APPL_NAME));
    } else if (var == "VERSION") {
        return (String(APPL_VERSION));
    } else if (var == "LIB_VERSION") {
        return (webSvcs.libVersion);
    } else if (var == "IP_ADDR") {
        return (WiFi.localIP().toString());
    } else if (var == "CONNECTION") {
        return (WiFi.SSID());
    } else if (var == "RSSI") {
        return (String(WiFi.RSSI()));
    } else if (var == "WIFI_MODE") {
        return getWiFiMode();
    } else if (var == "WIFI_AP_SSID") {
        return (WIFI_AP_SSID);
    }
    return String();
};

String indexPageMsgHandler(const JsonDocument& wsMsg) {
    Serial.print("COMMONMSGHANDLER: ");
    String msgType = String(wsMsg["msgType"]);
    if (msgType.equals("query")) {
        // NOP
    } else if (msgType.equals("saveConf")) {
        //// FIXME add code here
    }

    String msg = ", \"libVersion\": \"" + webSvcs.libVersion + "\"";
    msg += ", \"ipAddr\": \"" + WiFi.localIP().toString() + "\"";
    msg += ", \"connected\": \"" + WiFi.SSID() + "\"";
    msg += ", \"RSSI\": \"" + String(WiFi.RSSI()) + "\"";
    Serial.println(msg);
    return(msg);
};


WebPageDef commonIndexPage = {
    COMMON_HTML_PATH,
    COMMON_SCRIPTS_PATH,
    COMMON_STYLE_PATH,
    indexPageProcessor,
    indexPageMsgHandler
};


void halt() {
    while (true) {};
};

void config() {
    bool dirty = false;
    cs.open(CONFIG_PATH);

    if (!cs.configJsonDoc.containsKey("ssid")) {
        cs.configJsonDoc["ssid"] = configState.ssid;
        dirty = true;
    }
    if (!cs.configJsonDoc.containsKey("passwd")) {
        cs.configJsonDoc["passwd"] = configState.passwd;
        dirty = true;
    }
    if (dirty) {
        cs.saveConfig();
    }
    cs.printConfig();

    configState.ssid = cs.configJsonDoc["ssid"].as<String>();
    configState.passwd = cs.configJsonDoc["passwd"].as<String>();
}

void setup() {
    delay(500);
    Serial.begin(19200);
    delay(500);
    Serial.println("\nBEGIN");

    //// FIXME 
    if (false) {
        // clear the local file system
        cs.format();
    }
    config();

    Serial.println("Local Files:");
    cs.listFiles("/");

    wiFiConnect(configState.ssid, rot47(configState.passwd), WIFI_AP_SSID);

    if ((testMode & COMMON_GUI) == COMMON_GUI) {
//        webSvcs.addPage(COMMON_PAGE_PATH, COMMON_STYLE_PATH, COMMON_SCRIPT_PATH, processPage);
        if (!webSvcs.addPage(commonIndexPage)) {
            Serial.println("ERROR: halting");
            halt();
        }
    }
    cs.listFiles(COMMON_HTML_PATH);
    cs.listFiles(COMMON_STYLE_PATH);
    cs.listFiles(COMMON_SCRIPTS_PATH);

    if ((testMode & APPL_GUI) == APPL_GUI) {
//        applPagePath = APPL_PAGE_PATH;
    }
}

void loop() {
    webSvcs.run();
    if ((loopCnt % 5000000) == 0) {
        Serial.printf("loop: %d\n", loopCnt);
    }
    loopCnt++;
};
