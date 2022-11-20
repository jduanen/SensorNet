/***************************************************************************
 *
 * SensorNet WebServices library test
 *
 ***************************************************************************/

#include "wifi.h"
#include "WiFiUtilities.h"
#include "ConfigService.h"
#include "WebServices.h"


#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "2.0.0"
#define WIFI_AP_SSID        "WebServices"
#define WEB_SERVER_PORT     80
#define MAX_WS_MSG_SIZE     512

#define CONFIG_FILE_PATH    "/config.json"
#define CS_DOC_SIZE         1024

#define WS_HTML_PATH        "/index.html"
#define WS_STYLE_PATH       "/style.css"
#define WS_SCRIPTS_PATH     "/scripts.js"


typedef struct {
    String ssid;
    String passwd;

    bool        flag;
    uint32_t    intVal;
} ConfigState;


ConfigState configState = {
    String(WLAN_SSID),
    String(rot47(WLAN_PASS)),

    false,
    0
};

int loopCnt = 0;

ConfigService cs(CS_DOC_SIZE, CONFIG_FILE_PATH);

WebServices<MAX_WS_MSG_SIZE> webSvcs(APPL_NAME, WEB_SERVER_PORT);


void(* reboot)(void) = 0;

String pageProcessor(const String& var) {
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

String pageMsgHandler(const JsonDocument& wsMsg) {
    if (true) { //// TMP TMP TMP
        Serial.println("MSG:");
        if (false) {
            serializeJsonPretty(wsMsg, Serial);
        } else {
            serializeJson(wsMsg, Serial);
        }
        Serial.println("");
    }

    // handle changes in GUI; update HW and reflect it in the configState
    String msgType = String(wsMsg["msgType"]);
    if (msgType.equals("query")) {
        // NOP
    } else if (msgType.equals("setValues")) {
        configState.flag = wsMsg["flag"];
        configState.intVal = wsMsg["intVal"];
    } else if (msgType.equals("saveConf")) {
        String ssidStr = String(wsMsg["ssid"]);
        configState.ssid = ssidStr;
        SET_CONFIG(cs, "ssid", ssidStr);
        String passwdStr = String(wsMsg["passwd"]);
        configState.passwd = passwdStr;
        SET_CONFIG(cs, "passwd", passwdStr);

        bool flag = wsMsg["flag"];
        configState.flag = flag;
        SET_CONFIG(cs, "flag", flag);
        uint32_t intVal = wsMsg["intVal"];
        configState.intVal = intVal;
        SET_CONFIG(cs, "intVal", intVal);

        if (!cs.saveConfig()) {
            Serial.println("ERROR: Failed to write config file");
        }
        if (true) {
            Serial.println("Config File: XXXXXXXXXXXXXXXXXX");
            serializeJson(*(cs.doc), Serial);
            cs.listFiles("/");
            cs.printConfig();
            Serial.println("...\nXXXXXXXXXXXXXXXXX\n");
        }
    } else if (msgType.equalsIgnoreCase("reboot")) {
        Serial.println("REBOOTING...");
        reboot();
    } else if (msgType.equalsIgnoreCase("update")) {
        //// FIXME
        Serial.println("FIXME do the right thing here");
    } else {
        Serial.println("ERROR: unknown WS message type -- " + msgType);
    }

    // send contents of configState (which should reflect the state of the HW)
    String msg = ", \"libVersion\": \"" + webSvcs.libVersion + "\"";
    msg += ", \"ipAddr\": \"" + WiFi.localIP().toString() + "\"";
    msg += ", \"ssid\": \"" + WiFi.SSID() + "\"";
    msg += ", \"RSSI\": \"" + String(WiFi.RSSI()) + "\"";
    msg += ", \"flag\": \"" + String(configState.flag ? "true" : "false") + "\"";
    msg += ", \"intVal\": " + String(configState.intVal);
    if (false) {  //// TMP TMP TMP
        Serial.println(msg);
    }
    return(msg);
};

WebPageDef webPage = {
    WS_HTML_PATH,
    WS_SCRIPTS_PATH,
    WS_STYLE_PATH,
    pageProcessor,
    pageMsgHandler
};

void config() {
    bool flag;
    uint32_t intVal;
    bool dirty = false;

    // use value from defaults struct if a valid field not in config file
    INIT_CONFIG(cs, "ssid", configState.ssid);
    INIT_CONFIG(cs, "passwd", configState.passwd);
    INIT_CONFIG(cs, "flag", configState.flag);
    INIT_CONFIG(cs, "intVal", configState.intVal);
    if (dirty) {
        cs.saveConfig();
    }

    GET_CONFIG(configState.ssid, cs, "ssid", String);
    GET_CONFIG(configState.passwd, cs, "passwd", String);
    GET_CONFIG(configState.flag, cs, "flag", bool);
    GET_CONFIG(configState.intVal, cs, "intVal", unsigned int);
    if (false) {
        Serial.println("Config File: vvvvvvvvvvvvvvvvvvv");
        serializeJson(*(cs.doc), Serial);
        cs.listFiles("/");
        cs.printConfig();
        Serial.println("...\n^^^^^^^^^^^^^^^^^^^^^\n");
    }
}

void setup() {
    delay(500);
    Serial.begin(115200);
    delay(500);
    Serial.println("\nBEGIN");

    //// FIXME 
    if (false) {
        // clear the local file system
        cs.format();
    }

    //// TMP TMP TMP
    if (true) {
        // clear the config file
        Serial.print("Contents of config file: ");
        cs.printConfig();
        Serial.println("\nWrite empty json object to config file: " + String(CONFIG_FILE_PATH));
        deserializeJson(*(cs.doc), "{}");
        cs.saveConfig();
        Serial.print("Contents of empty config file: ");
        cs.printConfig();
        Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^");
    }

    Serial.println("Local Files:");
    cs.listFiles("/");

    config();

    wiFiConnect(configState.ssid, rot47(configState.passwd), WIFI_AP_SSID);

    if (!webSvcs.addPage(webPage)) {
        Serial.println("ERROR: failed to add web page; halting");
        while (true) {};
    }
    webSvcs.addPage(CONFIG_FILE_PATH, "application/json");

    webSvcs.updateClients();  //// FIXME remove this?

    Serial.println("READY");
};

void loop() {
    webSvcs.run();

    if ((loopCnt % 5000000) == 0) {
        Serial.printf("loop: %d\n", loopCnt);
    }
    loopCnt++;
};
