/*
*
* WaterHeater: WiFi, MQTT-based, temperature logging application
*
* Attach water temperature sensor to the output pipe of the water heater (after the recirculation pump)
*  - the ambient temperature sensor is to sense the temperature around the device
*
*/

#### TODO consider using ADC to monitor Vcc

#include <OneWire.h>
#include <DallasTemperature.h>
#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME        "WaterHeater"
#define APP_VERSION     "0.0.0"
#define REPORT_SCHEMA   "waterDegC:3.2f,ambientDegC:3.2f"

#define ONE_WIRE_BUS  2

// N.B. must correspond to how sensors are wired
#define WATER_TEMP_DEV_NUM    0
#define AMBIENT_TEMP_DEV_NUM  1

#define TOPIC_PREFIX    "/sensors/WaterHeater"

#define REPORT_INTERVAL 60000  // one report every 60 secs  #### TODO ????

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0

unsigned long lastReport = 0;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

//DeviceAddress thermDevAddr;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  int deviceCount = 0;
  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  ////sn.mqttSub(callback);

  sn.consolePrintln("Init temp sensors");
  sensors.begin();
  deviceCount = sensors.getDeviceCount();
  sn.consolePrintln("Found " + String(deviceCount) + " temp sensors");
  while (deviceCount != 2) {
    sn.consolePrintln("ERROR: Unable to find both temp sensors");
    sensors = DallasTemperature(&oneWire);
    sensors.begin();
    deviceCount = sensors.getDeviceCount();
  }
}

void loop() {
  String inMsg;
  String msg;
  float tempC;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  //// TODO add conditionals to power up/down the scale

  if (deltaT >= REPORT_INTERVAL) {
    sn.consolePrintln("Reading sensors");

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(WATER_TEMP_DEV_NUM);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg = String(tempC);
    } else {
      msg = "N/A";
    }
    tempC = sensors.getTempCByIndex(AMBIENT_TEMP_DEV_NUM);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg += "," + String(tempC);
    } else {
      msg += ",N/A";
    }

    sn.mqttPub(msg);
    sn.consolePrintln(msg);

    sn.consolePrintln("Sleep until next report");

    lastReport = now;
  }
}