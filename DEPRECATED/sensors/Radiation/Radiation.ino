/*
* Radiation Sensor: using GeigerKit with SBT-11(A) sensor
*/

#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME          "Radiation"
#define APP_VERSION       "1.3.0"
#define REPORT_SCHEMA     "CPM:d,uSv/h:.4f,Vcc:.2f"

#define TOPIC_PREFIX      "/sensors/Radiation"

#define REPORT_INTERVAL   60000  // one report every minute -- defined by sensor

#define MQTT_SERVER       "192.168.166.113"
#define MQTT_PORT         1883

#define GK_RESET          2   // GPIO0 aka D4, active low


unsigned int reportInterval = REPORT_INTERVAL;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);


void myCallback(char* topic, byte* payload, unsigned int length) {
  SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);
  if (cbMsg.handled == true) {
    sn.consolePrintln("Callback message handled by baseCallback");
    //// TODO test if trying to set reportInterval and issue warning that it can't be set
  } else {
    String respMsg = "Error: unhandled command: " + cbMsg.cmd;
    sn.consolePrintln(respMsg);
    sn.mqttPub(SensorNet::ERROR, respMsg);
  }
}

void setup() {
  sn.serialStart(&Serial, 9600, false);
  sn.serialStart(&Serial1, 9600, true);
  sn.consolePrintln(APP_NAME);

  pinMode(GK_RESET, OUTPUT);
  digitalWrite(GK_RESET, HIGH);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);
}

void loop() {
  String msg, inMsg;

  sn.mqttRun();

  // N.B. the sensor dictates the sample rate
  if (Serial.available() > 0) {
    inMsg = Serial.readStringUntil('\n');
    if (inMsg.length() < 1) {
      msg = "ERROR: read sensor timed out, resetting...";
      sn.consolePrintln(msg);
      sn.mqttPub(SensorNet::ERROR, msg);
      delay(60000);
      sn.systemReset();
    }
    sn.consolePrintln(inMsg);
    sn.mqttPub(SensorNet::DATA, inMsg);
  }
}
