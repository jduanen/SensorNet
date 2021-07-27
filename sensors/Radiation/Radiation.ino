/*
* Radiation Sensor: using GeigerKit with SBT-11(A) sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include "PMS.h"

#define APP_NAME        "Radiation"

#define TOPIC_PREFIX    "/sensors/Radiation"

#define REPORT_INTERVAL 60000  // one report every 60 secs

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

SensorNet sn(APP_NAME);

void callback(char* topic, byte* payload, unsigned int length) {
  sn.consolePrintln("Message arrived in topic: " + String(topic));

  sn.consolePrint("Message: ");
  for (int i = 0; i < length; i++) {
    sn.consolePrint(String(payload[i]));
  }
  sn.consolePrintln("");
}

void setup() {
  sn.serialStart(&Serial, 9600, false);
  sn.serialStart(&Serial1, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  ////sn.mqttSub(callback);
}

void loop() {
  String inMsg;

  sn.mqttRun();

  if (Serial.available() > 0) {
    inMsg = Serial.readStringUntil('\n');
    Serial.println("> " + inMsg);
    sn.mqttPub(inMsg);
  }
}
