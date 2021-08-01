/*
*
* BirdyNumNum: Temperature-calibrated, WiFi, MQTT-based, weight logging application
*
*/

#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SensorNet.h"
#include "wifi.h"


#define HX711_CLK     5
#define HX711_DOUT    4
#define ONE_WIRE_BUS  2
#define CALIBRATE_PIN 16

#define THERM_DEV_NUM 0

#define APP_NAME        "BirdyNumNum"
#define REPORT_SCHEMA   "degC:3.2f,light4d"

#define TOPIC_PREFIX    "/sensors/BNN"

#define REPORT_INTERVAL 60000  // one report every 60 secs  #### TODO ????

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define SCALE           2898.41

#define VERBOSE         0

unsigned long lastReport = 0;

SensorNet sn(APP_NAME);

DeviceAddress thermDevAddr;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

HX711 scale;

void setup() {
  pinMode(CALIBRATE_PIN, INPUT);
  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  ////sn.mqttSub(callback);

  sn.consolePrintln("Init temp sensor");
  sensors.begin();
  if (!sensors.getAddress(thermDevAddr, THERM_DEV_NUM)) {
    sn.consolePrintln("ERROR: Unable to find address of thermometer device");
  }

  sn.consolePrintln("Init scale");
  scale.begin(HX711_DOUT, HX711_CLK);
  scale.reset();
  while (!scale.wait_ready_retry(100, 100)) {
    sn.consolePrintln("Scale not ready");
    delay(1000);
  }
  sn.consolePrintln("Calibrating");
  scale.tare();
  if (digitalRead(CALIBRATE_PIN) == 0) {
    sn.consolePrintln("Empty scale, then hit enter");
    //// TODO add this idiom to the SensorNet library
    while (!Serial.available());
    while (Serial.available()) Serial.read();
    sn.consolePrintln("Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
    sn.consolePrintln("Put 50g weight on scale, then hit enter");
    //// TODO add this idiom to the SensorNet library
    while (!Serial.available());
    while (Serial.available()) Serial.read();
    scale.calibrate_scale(50, 5);
    sn.consolePrintln("Calibrated");
    sn.consolePrintln("Scale: " + String(scale.get_scale()) + ", Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
  } else {
    scale.set_scale(SCALE);
    sn.consolePrintln("Set Scale=" + String(SCALE) + ", Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
  }
}

void loop() {
  String inMsg;
  String msg;
  float tempC;
  int light;
  float units;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  //// TODO add conditionals to power up/down the scale

  if (deltaT >= REPORT_INTERVAL) {
    sn.consolePrintln("Reading sensors");

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg = String(tempC);
    } else {
      msg = "N/A";
    }

    light = analogRead(A0);
    msg += "," + String(light);

    units = scale.get_units(5);
    msg += "," + String(units);

    sn.mqttPub(msg);
    sn.consolePrintln(msg);

    sn.consolePrintln("Sleep until next report");

    lastReport = now;
  }
}
