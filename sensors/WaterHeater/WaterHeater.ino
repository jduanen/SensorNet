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
//#include "WebServices.h"


#define VERBOSE                     1

#define APP_NAME                    "WaterHeater"
#define APP_VERSION                 "2.0.0"
#define REPORT_SCHEMA               "waterDegC:3.2f,ambientDegC:3.2f"

#define CONFIG_PATH         "/config.json"

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


SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA, COMMAND_NAMES);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors;
DeviceAddress waterThermometer, ambientThermometer;


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
        }
    } else {
        if (cbMsg.cmd.equalsIgnoreCase("Precision")) {
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
    cs.saveConfig();
    if (VERBOSE) {
        cs.listFiles(CONFIG_PATH);
        cs.printConfig();
    }
}

void setup() {
    int deviceCount = 0;
    String msg;

    sn.serialStart(&Serial, 19200, true);
    sn.consolePrintln(APP_NAME);

    config();
    sn.reportInterval = configState.reportInterval;

    sn.wifiStart(configState.ssid, rot47(configState.passwd));

    sn.mqttSetup(configState.mqttServer, configState.mqttPort, TOPIC_PREFIX, myCallback);

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
}

void loop() {
    String inMsg;
    String msg;
    float tempC;
    unsigned long now = millis();
    unsigned long deltaT = now - sn.lastReport;

    sn.mqttRun();

    if (deltaT >= sn.reportInterval) {
        sn.consolePrintln("Reading sensors");

        sensors.requestTemperatures();
        tempC = sensors.getTempC(waterThermometer);
        if (tempC != DEVICE_DISCONNECTED_C) {
            msg = String(tempC);
        } else {
            msg = "N/A";
        }
        tempC = sensors.getTempC(ambientThermometer);
        if (tempC != DEVICE_DISCONNECTED_C) {
            msg += "," + String(tempC);
        } else {
            msg += ",N/A";
        }

        sn.mqttPub(SensorNet::DATA, msg);
        sn.consolePrintln(msg);
        sn.consolePrintln("Sleep until next report");

        sn.lastReport = now;
    }
}
