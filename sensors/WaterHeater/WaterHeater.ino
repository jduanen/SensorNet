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
#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME                    "WaterHeater"
#define APP_VERSION                 "1.2.1"
#define REPORT_SCHEMA               "waterDegC:3.2f,ambientDegC:3.2f"

#define ONE_WIRE_BUS                2

// N.B. must correspond to how sensors are wired
#define AMBIENT_TEMP_DEV_NUM        0
#define WATER_TEMP_DEV_NUM          1

#define DEF_TEMPERATURE_PRECISION   12

#define TOPIC_PREFIX                "/sensors/WaterHeater"

#define DEF_REPORT_INTERVAL         60000  // one report every minute

#define MQTT_SERVER                 "192.168.166.113"
#define MQTT_PORT                   1883

#define VERBOSE                     1

#define MAX_CMD_LEN                 16
#define MAX_VAL_LEN                 16


SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors;
DeviceAddress waterThermometer, ambientThermometer;


void myCallback(char* topic, byte* payload, unsigned int length) {
    String respMsg;
    char msgType = SensorNet::RESPONSE;
    SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);

    if (cbMsg.handled == true) {
        sn.consolePrintln("Callback message handled by baseCallback");
        // N.B. you can do other stuff in addition to what's done in the base hander here
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
}

void setup() {
    int deviceCount = 0;
    String msg;

    sn.serialStart(&Serial, 19200, true);
    sn.consolePrintln(APP_NAME);

    sn.wifiStart(WLAN_SSID, WLAN_PASS);

    sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);

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
    if (!sensors.setResolution(ambientThermometer, DEF_TEMPERATURE_PRECISION) ||
        !sensors.setResolution(waterThermometer, DEF_TEMPERATURE_PRECISION)) {
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
