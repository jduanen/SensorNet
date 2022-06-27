/***************************************************************************
 *
 * SensorNet WebServices library test
 *
 * Notes:
 *   - ?
 *
 ***************************************************************************/

#include "WebServices.h"
#include "wifi.h"


#define VERBOSE             1

#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "1.0.0"
#define WEB_SERVER_PORT     80
#define WIFI_AP_SSID        "prcDisplay"
#define MAX_WIFI_RETRIES    8 //64

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


//// TODO add this to a common library
String rot47(String str) {
  String outStr = "";
  char oldChr, newChr;

  for (int i = 0; (i < str.length()); i++) {
    oldChr = str.charAt(i);
    if ((oldChr >= '!') && (oldChr <= 'O')) {
      newChr = ((oldChr + 47) % 127);
    } else {
      if ((oldChr >= 'P') && (oldChr <= '~')) {
        newChr = ((oldChr - 47) % 127);
      } else {
        newChr = oldChr;
      }
    }
    outStr.concat(newChr);
  }
  return(outStr);
}

WiFiMode_t wifiMode;
IPAddress softIPA;

typedef struct {
  String ssid;
  String passwd;
} ConfigState;

ConfigState configState = {
  String(WLAN_SSID),
  String(rot47(WLAN_PASS)),
};


const char* ssid = WLAN_SSID;
const char* password = WLAN_PASS;

WebServices webSvcs(APPL_NAME, WEB_SERVER_PORT);

////int testMode = FW_UPDATE;
int testMode = FW_UPDATE | COMMON_GUI;
////int testMode = FW_UPDATE | COMMON_GUI | APPL_GUI;

int loopCnt = 0;


//// FIXME Move this to a library
String wifiStatusToString(wl_status_t status) {
    String s = "WIFI_UNKNOWN: " + String(status);
    switch (status) {
        case WL_NO_SHIELD: s = "WIFI_NO_SHIELD";
        case WL_IDLE_STATUS: s = "WIFI_IDLE_STATUS";
        case WL_NO_SSID_AVAIL: s = "WIFI_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED: s = "WIFI_SCAN_COMPLETED";
        case WL_CONNECTED: s = "WIFI_CONNECTED";
        case WL_CONNECT_FAILED: s = "WIFI_CONNECT_FAILED";
        case WL_CONNECTION_LOST: s = "WIFI_CONNECTION_LOST";
        case WL_DISCONNECTED: s = "WIFI_DISCONNECTED";
    }
    return(s);
}


String commonHandlerWrapper(StaticJsonDocument<WS_JSON_DOC_SIZE> wsMsg) {
    return(webSvcs.commonMsgHandler(wsMsg));
}


/*
bool fileExists(String path) {
    if (!LittleFS.begin()) {
        Serial.println("ERROR: failed to mount LittleFS");
        return false;
    }
    File f = LittleFS.open(path, "r");
    return !f;
}
*/

//// TODO make this take indent level arg
//// FIXME move this to WebServices library
#define NUM_ITEMS(arr)  ((unsigned int)(sizeof(arr) / sizeof(arr[0])))

void listFiles(String dirPath) {
    //// TODO add '/' if dirPath doesn't end with one
    Dir d = LittleFS.openDir(dirPath);
    while (d.next()) {
        if (d.isFile()) {
            Serial.println("File: " + dirPath + d.fileName());
        } else if (d.isDirectory()) {
            String p = dirPath + d.fileName() + "/";
            Serial.println("Dir: " + p);
            listFiles(p);
        } else {
            Serial.println("Unknown: " + d.fileName());
        }
    }
}

//// FIXME this should be in a common part of the WS library
const char modes[][4] = {"OFF", "STA", "A_P", "APS"};

String proc(const String& var) {
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
    return (modes[wifiMode]);
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

    //// FIXME this belongs in a common part of the WS library
    wifiMode = WIFI_STA;
    WiFi.mode(wifiMode);
    //WiFi.begin(configState.ssid, rot47(configState.passwd));
    WiFi.begin(configState.ssid, rot47(configState.passwd));
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi.." + wifiStatusToString(WiFi.status()));
        if (i++ > MAX_WIFI_RETRIES) {
            Serial.println("Switch to AP mode: ");
            wifiMode = WIFI_AP;
            WiFi.mode(wifiMode);
            WiFi.softAP(WIFI_AP_SSID);
            break;
        }
        //// TODO figure out how to trigger this
        if (false) {
            if (i++ > MAX_WIFI_RETRIES) {
                Serial.println("Using fallback WiFi parameters");
                configState.ssid = WLAN_SSID;
                configState.passwd = rot47(WLAN_PASS);
                WiFi.begin(configState.ssid, rot47(configState.passwd));
                delay(1000);
                i = 0;
            }
        }
    }
    if (wifiMode == WIFI_STA) {
        Serial.println("\nWiFi Station Mode");
        Serial.println("Connected to " + WiFi.SSID());
        Serial.println("IP address: " + WiFi.localIP().toString());
    }
    if (wifiMode == WIFI_AP) {
        Serial.println("\nWiFi Access Point Mode");
        Serial.println("AP SSID: " + String(WIFI_AP_SSID));
        softIPA = WiFi.softAPIP();
        Serial.println("AP IP Address: " + softIPA.toString());
    }
    Serial.println("RSSI: " + String(WiFi.RSSI()));

    //// FIXME
    if (false) {
        LittleFS.format();
    }
    Serial.println("List /");
    listFiles("/");
    Serial.println("List /common/");
    listFiles("/common/");

    if ((testMode & COMMON_GUI) == COMMON_GUI) {
        /*
        String paths[] = {COMMON_PAGE_PATH, COMMON_STYLE_PATH, COMMON_SCRIPT_PATH};
        for (int i = 0; (i < NUM_ITEMS(paths)); i++) {
            Serial.print(paths[i]);
            if (LittleFS.exists(paths[i]) == false) {
                Serial.println(": file not found");
            } else {
                Serial.println(": file exists");
            }
        }
        */
        webSvcs.addPage(COMMON_PAGE_PATH, COMMON_STYLE_PATH, COMMON_SCRIPT_PATH, proc);
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
