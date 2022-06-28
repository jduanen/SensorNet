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
#include "LfsUtilities.h"


#define VERBOSE             1

#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "1.0.0"
#define WEB_SERVER_PORT     80
#define WIFI_AP_SSID        "prcDisplay"

//#define USER_NAME           "name"
//#define PASSWD              "passwd"

/*
#define CONFIG_PATH         "/config.json"
#define CONFIG_FILE         (1 << 1)    // test configuration feature
    if ((testMode & CONFIG_FILE) == CONFIG_FILE) {
        confPath = CONFIG_PATH;
    }
*/

#define COMMON_PAGE_PATH    "/index.html"
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

String processPage(const String& var) {
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
}

void setup() {
    delay(500);
    Serial.begin(19200);
    delay(500);
    Serial.println("\nBEGIN");

    //// FIXME use the config library
    String confPath = "";

    //// FIXME 
    if (false) {
        // clear the local file system
        formatLFS();
    }

    Serial.println("Local files: ");
    listFiles("/");

    wiFiConnect(configState.ssid, rot47(configState.passwd), WIFI_AP_SSID);

    if ((testMode & COMMON_GUI) == COMMON_GUI) {
        webSvcs.addPage(COMMON_PAGE_PATH, COMMON_STYLE_PATH, COMMON_SCRIPT_PATH, processPage);
    }

    if ((testMode & APPL_GUI) == APPL_GUI) {
//        applPagePath = APPL_PAGE_PATH;
    }
}

void loop() {
    webSvcs.run();
    if ((loopCnt % 500000) == 0) {
        Serial.println("loop: " + String(loopCnt));
    }
    loopCnt++;
};
