/***************************************************************************
 *
 * SensorNet WebServices Library Test
 *
 ***************************************************************************/

#include "wifi.h"
#include "WiFiUtilities.h"
#include "ConfigService.h"
#include "WebServices.h"


#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "2.1.0"
#define WIFI_AP_SSID        "WebServices"
#define WEB_SERVER_PORT     80
#define MAX_WS_MSG_SIZE     1024

#define CONFIG_FILE_PATH    "/config.json"
#define CS_DOC_SIZE         1024

#define WS_HTML_PATH        "/index.html"
#define WS_STYLE_PATH       "/style.css"
#define WS_SCRIPTS_PATH     "/scripts.js"

#define NUM_TUPLES          16


typedef struct {
    uint32_t start;
    uint32_t end;
} MyTuple;

typedef struct {
    String      ssid;
    String      passwd;

    bool        flag;
    uint32_t    intVal;

    String      str;
    uint32_t    tuples[NUM_TUPLES][2];
} ConfigState;


ConfigState configState = {
    String(WLAN_SSID),
    String(rot47(WLAN_PASS)),

    false,
    0,

    "This is a string",
    {
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080},
        {0x808080, 0x808080}
    }
};

int loopCnt = 0;

ConfigService cs(CS_DOC_SIZE, CONFIG_FILE_PATH);

WebServices<MAX_WS_MSG_SIZE> webSvcs(APPL_NAME, WEB_SERVER_PORT);


void halt() {
    Serial.println("HALT");
    while (true) {
        wdt_reset();
        delay(100);
    }
};

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
    } else if (var == "NUM_TUPLES") {
        return (String(NUM_TUPLES));
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
        configState.str = String(wsMsg["str"]);
        copyArray(wsMsg["tuples"], configState.tuples);
        printTuples("Tuples:", configState.tuples);
    } else if (msgType.equals("saveConf")) {
        String jsonStr;
        JSON_START(jsonStr);
        jsonStr.concat("\"ssid\": \"" + configState.ssid + "\", ");
        jsonStr.concat("\"passwd\": \"" + configState.passwd + "\", ");
        jsonStr.concat("\"flag\": " + String(configState.flag ? true : false) + ", ");
        jsonStr.concat("\"intVal\": " + String(configState.intVal) + ", ");
        jsonStr.concat("\"str\": \"" + configState.str + "\", ");

        jsonStr.concat("\"tuples\": [");
        for (int i = 0; (i < NUM_TUPLES); i++) {
            if (i > 0) {
                jsonStr.concat(", ");
            }
            jsonStr.concat("[" + String(configState.tuples[i][0]) + ", " + String(configState.tuples[i][1]) + "]");
        }
        jsonStr.concat("]}");
        JSON_END(jsonStr);
        /*
        jsonStr.concat("\"\": \"" + configState. + "\", ");
        jsonStr.concat("\"\": " + String(configState. ? true : false) + ", ");
        jsonStr.concat("\"\": " + String(configState.) + ", ");
        */

        if (!cs.setConfig(jsonStr)) {
            Serial.println("ERROR: failed to save config");
        }
    } else if (msgType.equalsIgnoreCase("reboot")) {
        Serial.println("REBOOTING...");
        reboot();
    } else if (msgType.equalsIgnoreCase("update")) {
        //// FIXME
        Serial.println("FIXME do the right thing here");
    } else {        Serial.println("ERROR: unknown WS message type -- " + msgType);
    }

    // send contents of configState (which should reflect the state of the HW)
    String msg = ", \"libVersion\": \"" + webSvcs.libVersion + "\"";
    msg.concat(", \"ipAddr\": \""); msg.concat(WiFi.localIP().toString() + "\"");
    msg.concat(", \"ssid\": \""); msg.concat(WiFi.SSID() + "\"");
    msg.concat(", \"passwd\": \""); msg.concat(configState.passwd + "\"");
    msg.concat(", \"RSSI\": "); msg.concat(WiFi.RSSI());
    msg.concat(", \"flag\": "); msg.concat(configState.flag);
    msg.concat(", \"intVal\": "); msg.concat(configState.intVal);
    msg.concat(", \"str\": \""); msg.concat(configState.str + "\"");
    msg.concat(", \"tuples\": "); msg.concat(tuples2String(configState.tuples));
    if (false) {  //// TMP TMP TMP
        Serial.print("msg: ");
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

void printTuples(String hdr, uint32_t tuples[][2]) {
    Serial.print(hdr);
    //// use forall/iterator instead?
    for (int i = 0; (i < NUM_TUPLES); i++) {
        if (i > 0) {
            Serial.print(", ");
        }
        Serial.print("[0x" + String(tuples[i][0], HEX) + ", 0x" + String(tuples[i][1], HEX) + "]");
    }
    Serial.println("]");
};

String tuples2String(uint32_t tuples[][2]) {
    String ccStr = "[";
    for (int i = 0; (i < NUM_TUPLES); i++) {
        if (i != 0) {
            ccStr += ",";
        }
        ccStr.concat("[" + String(tuples[i][0]) + "," +  String(tuples[i][1]) + "]");
    }
    ccStr.concat("]");
    return ccStr;
};

void config() {
    bool flag;
    uint32_t intVal;

    configState.ssid = cs.getConfigValue<String>("ssid", configState.ssid);
    configState.passwd = cs.getConfigValue<String>("passwd", configState.passwd);
    configState.flag = cs.getConfigValue<bool>("flag", configState.flag);
    configState.intVal = cs.getConfigValue<int>("intVal", configState.intVal);
    configState.str = cs.getConfigValue<String>("str", configState.str);

    uint16_t numTuples = (*(cs.docPtr))["tuples"].size();
//    Serial.println("> #Tuples: " + String(numTuples) + ", OVFL: " + String(cs.docPtr->overflowed()) + ", VALID: " + cs.validEntry("tuples"));
    if (!cs.validEntry("tuples")) {
        cs.docPtr->createNestedArray("tuples");
        numTuples = 0;
    }
//    Serial.println(">> #Tuples: " + String(numTuples) + ", OVFL: " + String(cs.docPtr->overflowed()) + ", VALID: " + cs.validEntry("tuples"));
    if (numTuples != NUM_TUPLES) {
        Serial.println("ERROR: wrong number of tuples");
        //// FIXME
    }
    if (cs.validEntry("tuples") && (numTuples == NUM_TUPLES)) {
        copyArray((*(cs.docPtr))["tuples"], configState.tuples);
        if (cs.docPtr->overflowed()) {
            Serial.println("ERROR: overflowed config JsonDocument");
        }
        numTuples = (*(cs.docPtr))["tuples"].size();
        if (numTuples != NUM_TUPLES) {
            Serial.println("ERROR: incorrect number of tuples: " + String(numTuples));
        }
    }
//    Serial.println(">>> #Tuples: " + String(numTuples) + ", OVFL: " + String(cs.docPtr->overflowed()) + ", VALID: " + cs.validEntry("tuples"));
}

void setup() {
    delay(500);
    Serial.begin(115200);
    delay(500);
    Serial.print("\nBEGIN: "); Serial.println(APPL_NAME);
    cs.printConfigFile();

    if (false) {  //// TMP TMP TMP
        // clear the local file system
        cs.formatFS();
    }

    if (false) {  //// TMP TMP TMP
        // disregard the contents of the saved config file
        cs.initializeConfig();
    }

    //// TMP TMP TMP
    if (false) {
        // clear the config file
        Serial.print("Contents of config file: ");
        cs.printConfigFile();
        Serial.println("\nWrite empty json object to config file: " + String(CONFIG_FILE_PATH));
        deserializeJson(*(cs.docPtr), "{}");
        cs.saveConfig();
        Serial.print("Contents of empty config file: ");
        cs.printConfigFile();
        Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^");
    }

    Serial.println("Local Files:");
    cs.listFiles("/");

    config();
    cs.printConfigFile();  //// TMP TMP TMP

    wiFiConnect(configState.ssid, rot47(configState.passwd), WIFI_AP_SSID);

    if (!webSvcs.addPage(webPage)) {
        Serial.println("ERROR: failed to add web page; halting");
        while (true) {};
    }
    webSvcs.addPage(CONFIG_FILE_PATH, "application/json");

    //webSvcs.updateClients();  //// FIXME remove this?

    Serial.println("READY");
};

void loop() {
    webSvcs.run();

    if ((loopCnt % 5000000) == 0) {
        Serial.printf("loop: %d\n", loopCnt);
    }
    loopCnt++;
};
