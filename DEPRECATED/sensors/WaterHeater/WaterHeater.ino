/*
*
* WaterHeater: WiFi, MQTT-based, temperature logging application
*
* Attach water temperature sensor to the output pipe of the water heater (after the recirculation pump)
*  - the ambient temperature sensor is to sense the temperature around the device
*
*/

//// TODO consider using ADC to monitor Vcc

#include <OneWire.h>
#include <DallasTemperature.h>

#include "wifi.h"
#include "SensorNet.h"
#include "WiFiUtilities.h"
#include "ConfigService.h"
#include "WebServices.h"


#define VERBOSE                     1

#define APPL_NAME                   "WaterHeater"
#define APPL_VERSION                "2.0.0"
#define REPORT_SCHEMA               "waterDegC:3.2f,ambientDegC:3.2f"

#define CONFIG_PATH                 "/config.json"

#define WIFI_AP_SSID                "waterHeater"
#define WEB_SERVER_PORT             80

#define ONE_WIRE_BUS                2

// N.B. must correspond to how sensors are wired
#define AMBIENT_TEMP_DEV_NUM        0
#define WATER_TEMP_DEV_NUM          1

#define DEF_TEMPERATURE_PRECISION   12

#define TOPIC_PREFIX                "/sensors/WaterHeater"

#define DEF_REPORT_INTERVAL         60000  // one report every minute

#define MQTT_SERVER                 "192.168.166.113"
#define MQTT_PORT                   1883

#define MAX_CMD_LEN                 16
#define MAX_VAL_LEN                 16

#define COMMAND_NAMES               "Precision"

#define WH_HTML_PATH                "/index.html"
#define WH_STYLE_PATH               "/style.css"
#define WH_SCRIPTS_PATH             "/scripts.js"


typedef struct {
    String    ssid;
    String    passwd;
    String    mqttServer;
    uint16_t  mqttPort;
    uint32_t  reportInterval;
    uint8_t   tempPrecision;
} ConfigState;

ConfigState configState = {
    String(WLAN_SSID),
    String(rot47(WLAN_PASS)),
    MQTT_SERVER,
    MQTT_PORT,
    DEF_REPORT_INTERVAL,
    DEF_TEMPERATURE_PRECISION
};

float waterTemp, ambientTemp;

SensorNet sn(APPL_NAME, APPL_VERSION, REPORT_SCHEMA, COMMAND_NAMES);

WebServices webSvcs(APPL_NAME, WEB_SERVER_PORT);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors;
DeviceAddress waterThermometer, ambientThermometer;


void(* reboot)(void) = 0;

String webpageProcessor(const String& var) {
    if (var == "APPL_NAME") {
        return (String(APPL_NAME));
    } else if (var == "VERSION") {
        return (String(APPL_VERSION));
    } else if (var == "LIB_VERSION") {
        return (webSvcs.libVersion);
    } else if (var == "IP_ADDR") {
        return (WiFi.localIP().toString());
    } else if (var == "SSID") {
        return (configState.ssid);
    } else if (var == "RSSI") {
        return (String(WiFi.RSSI()));
    } else if (var == "WIFI_MODE") {
        return getWiFiMode();
    } else if (var == "WIFI_AP_SSID") {
        return (WIFI_AP_SSID);
    } else if (var == "MQTT_SERVER") {
        return (String(configState.mqttServer));
    } else if (var == "MQTT_PORT") {
        return (String(configState.mqttPort));
    } else if (var == "RATE") {
        return (String(sn.reportInterval));
    } else if (var == "PRECISION") {
        return (String(configState.tempPrecision));
    } else if (var == "WATER_TEMP") {
        return (String(waterTemp));
    } else if (var == "AMBIENT_TEMP") {
        return (String(ambientTemp));
    }
    return String();
};

String webpageMsgHandler(const JsonDocument& wsMsg) {
    String msgType = String(wsMsg["msgType"]);
    if (msgType.equals("query")) {
        // NOP
    } else if (msgType.equals("rate")) {
        unsigned int rate = wsMsg["rate"];
        sn.reportInterval = rate;
        configState.reportInterval = rate;
    } else if (msgType.equals("precision")) {
        unsigned int precision = wsMsg["precision"];
        configState.tempPrecision = precision;
        sn.consolePrintln("Set precision to: " + precision);
        sensors.setResolution(ambientThermometer, precision);
        sensors.setResolution(waterThermometer, precision);
    } else if (msgType.equals("saveConf")) {
        String ssid = String(wsMsg["ssid"]);
        configState.ssid = ssid;
        cs.configJsonDoc["ssid"] = ssid;
        String passwd = String(wsMsg["passwd"]);
        configState.passwd = passwd;
        cs.configJsonDoc["passwd"] = passwd;

        String mqttServer = String(wsMsg["mqttServer"]);
        configState.mqttServer = mqttServer;
        cs.configJsonDoc["mqttServer"] = mqttServer;
        unsigned int mqttPort = wsMsg["mqttPort"];
        configState.mqttPort = mqttPort;
        cs.configJsonDoc["mqttPort"] = mqttPort;

        unsigned int rate = wsMsg["rate"];
        configState.reportInterval = rate;
        cs.configJsonDoc["reportInterval"] = rate;

        unsigned int precision = wsMsg["precision"];
        configState.tempPrecision = precision;
        cs.configJsonDoc["tempPrecision"] = precision;
    
        cs.saveConfig();
    } else if (msgType.equals("reboot")) {
        sn.consolePrintln("REBOOTING...");
        reboot();
    }

    String msg = ", \"libVersion\": \"" + webSvcs.libVersion + "\"";
    msg += ", \"ipAddr\": \"" + WiFi.localIP().toString() + "\"";
    msg += ", \"ssid\": \"" + configState.ssid + "\"";
    msg += ", \"passwd\": \"" + configState.passwd + "\"";
    msg += ", \"RSSI\": \"" + String(WiFi.RSSI()) + "\"";
    msg += ", \"mqttServer\": \"" + configState.mqttServer  + "\"";
    msg += ", \"mqttPort\": \"" + String(configState.mqttPort)  + "\"";
    msg += ", \"rate\": \"" + String(sn.reportInterval) + "\"";
    msg += ", \"precision\": \"" + String(configState.tempPrecision) + "\"";
    msg += ", \"waterTemp\": \"" + String(waterTemp) + "\"";
    msg += ", \"ambientTemp\": \"" + String(ambientTemp) + "\"";
    //Serial.println(msg);
    return(msg);
};


WebPageDef webPage = {
    WH_HTML_PATH,
    WH_SCRIPTS_PATH,
    WH_STYLE_PATH,
    webpageProcessor,
    webpageMsgHandler
};


void myCallback(char* topic, byte* payload, unsigned int length) {
    String respMsg;
    char msgType = SensorNet::RESPONSE;
    SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);
    bool dirty = false;

    if (cbMsg.handled == true) {
        if (cbMsg.cmd.equalsIgnoreCase("rate") && (cbMsg.val != NULL)) {
            sn.reportInterval = cbMsg.val.toInt();
            configState.reportInterval = sn.reportInterval;
            cs.configJsonDoc["reportInterval"] = configState.reportInterval;
            dirty = true;
        } else if (cbMsg.cmd.equalsIgnoreCase("mqttServer") && (cbMsg.val != NULL)) {
            configState.mqttServer = cbMsg.val;
            cs.configJsonDoc["mqttServer"] = configState.mqttServer;
            dirty = true;
        } else if (cbMsg.cmd.equalsIgnoreCase("mqttPort") && (cbMsg.val != NULL)) {
            configState.mqttPort = cbMsg.val.toInt();
            cs.configJsonDoc["mqttPort"] = configState.mqttPort;
            dirty = true;
        }
    } else {
        if (cbMsg.cmd.equalsIgnoreCase("precision")) {
            uint8_t precision;
            if (cbMsg.val != NULL) {
                precision = cbMsg.val.toInt();
                if ((precision < 9) || (precision > 12)) {
                    sn.consolePrintln("ERROR: Invalid precision value: " + cbMsg.val);
                } else {
                    sn.consolePrintln("Set precision to: " + cbMsg.val);
                    sensors.setResolution(ambientThermometer, precision);
                    sensors.setResolution(waterThermometer, precision);

                    configState.tempPrecision = precision;
                    cs.configJsonDoc["tempPrecision"] = configState.tempPrecision;
                    dirty = true;
                }
            }
            uint8_t ambPrec = sensors.getResolution(ambientThermometer);
            uint8_t watPrec = sensors.getResolution(waterThermometer);
            if (ambPrec != watPrec) {
                respMsg = "ERROR: precision mismatch: " + String(ambPrec) + ", " + String(watPrec);
                sn.consolePrintln(respMsg);
                sn.mqttPub(SensorNet::ERROR, respMsg);
            } else {
               precision = watPrec;
            }
            respMsg = "Precision=" + String(precision);
            sn.consolePrintln(respMsg);
            sn.mqttPub(SensorNet::RESPONSE, respMsg);
        } else {
            respMsg = "ERROR: unknown command (" + cbMsg.cmd + ")";
            msgType = SensorNet::ERROR;
        }
        sn.consolePrintln("Response Message: " + respMsg);
        sn.mqttPub(msgType, respMsg);
    }
    if (dirty) {
        cs.saveConfig();
        if (VERBOSE) {
            cs.listFiles(CONFIG_PATH);
            cs.printConfig();
        }
    }
}

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
    if (!cs.configJsonDoc.containsKey("mqttServer")) {
        cs.configJsonDoc["mqttServer"] = configState.mqttServer;
        dirty = true;
    }
    if (!cs.configJsonDoc.containsKey("mqttPort")) {
        cs.configJsonDoc["mqttPort"] = configState.mqttPort;
        dirty = true;
    }
    if (!cs.configJsonDoc.containsKey("reportInterval")) {
        cs.configJsonDoc["reportInterval"] = configState.reportInterval;
        dirty = true;
    }
    if (!cs.configJsonDoc.containsKey("tempPrecision")) {
        cs.configJsonDoc["tempPrecision"] = configState.tempPrecision;
        dirty = true;
    }
    if (dirty) {
        cs.saveConfig();
    }

    configState.ssid = cs.configJsonDoc["ssid"].as<String>();
    configState.passwd = cs.configJsonDoc["passwd"].as<String>();
    configState.mqttServer = cs.configJsonDoc["mqttServer"].as<String>();
    configState.mqttPort = cs.configJsonDoc["mqttPort"].as<unsigned int>();
    configState.reportInterval = cs.configJsonDoc["reportInterval"].as<unsigned int>();
    configState.tempPrecision = cs.configJsonDoc["tempPrecision"].as<unsigned int>();
    if (VERBOSE) {
        sn.consolePrintln("Config File:");
        cs.listFiles(CONFIG_PATH);
        cs.printConfig();
    }
}

void setup() {
    int deviceCount = 0;
    String msg;

    sn.serialStart(&Serial, 19200, true);
    sn.consolePrintln(APPL_NAME);

    //// FIXME 
    if (false) {
        // clear the local file system
        cs.format();
    }

    if (VERBOSE) {
        sn.consolePrintln("Local Files:");
        cs.listFiles("/");
    }

    config();
    sn.reportInterval = configState.reportInterval;

    wiFiConnect(configState.ssid, rot47(configState.passwd), WIFI_AP_SSID);

    sn.mqttSetup(configState.mqttServer, configState.mqttPort, TOPIC_PREFIX, myCallback);

    if (!webSvcs.addPage(webPage)) {
        sn.consolePrintln("ERROR: failed to add common page; continuing anyway");
    }

    sn.consolePrintln("Init temp sensors");
    sensors = DallasTemperature(&oneWire);
    sensors.begin();
    deviceCount = sensors.getDeviceCount();
    sn.consolePrintln("Found " + String(deviceCount) + " temp sensors");

    if (deviceCount != 2) {
        msg = "ERROR: Unable to find both temp sensors";
        sn.consolePrintln(msg);
        sn.mqttPub(SensorNet::ERROR, msg);
        delay(60000);
        sn.systemReset();
    }
    if (!sensors.getAddress(ambientThermometer, AMBIENT_TEMP_DEV_NUM) ||
        !sensors.getAddress(waterThermometer, WATER_TEMP_DEV_NUM)) {
        msg = "ERROR: Unable to get both temp sensor addresses";
        sn.consolePrintln(msg);
        sn.mqttPub(SensorNet::ERROR, msg);
        delay(60000);
        sn.systemReset();
    }
    if (!sensors.setResolution(ambientThermometer, configState.tempPrecision) ||
        !sensors.setResolution(waterThermometer, configState.tempPrecision)) {
        msg = "ERROR: Failed to set temperature precision";
        sn.consolePrintln(msg);
        sn.mqttPub(SensorNet::ERROR, msg);
        delay(60000);
        sn.systemReset();
    }

    // read and discard first values from both sensors
    (void)sensors.getTempC(waterThermometer);
    (void)sensors.getTempC(ambientThermometer);
    sn.consolePrintln("READY");
}

void loop() {
    String inMsg;
    String msg;
    unsigned long now = millis();
    unsigned long deltaT = now - sn.lastReport;

    sn.mqttRun();

    if (deltaT >= sn.reportInterval) {
        sn.consolePrintln("Reading sensors");

        sensors.requestTemperatures();
        waterTemp = sensors.getTempC(waterThermometer);
        if (waterTemp != DEVICE_DISCONNECTED_C) {
            msg = String(waterTemp);
        } else {
            msg = "N/A";
        }
        ambientTemp = sensors.getTempC(ambientThermometer);
        if (ambientTemp != DEVICE_DISCONNECTED_C) {
            msg += "," + String(ambientTemp);
        } else {
            msg += ",N/A";
        }

        sn.mqttPub(SensorNet::DATA, msg);
        sn.consolePrintln(msg);
        webSvcs.updateClients();
        sn.consolePrintln("Sleep until next report");

        sn.lastReport = now;
    }
}
