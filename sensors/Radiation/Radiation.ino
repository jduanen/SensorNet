/*
* Radiation Sensor: using GeigerKit with SBT-11(A) sensor
*/

#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME        "Radiation"
#define APP_VERSION     "1.1.0"
#define REPORT_SCHEMA   "CPM:d,uSv/h:.4f,Vcc:.2f"

#define TOPIC_PREFIX    "/sensors/Radiation"

#define REPORT_INTERVAL 60000  // one report every minute -- defined by sensor

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883


unsigned int reportInterval = REPORT_INTERVAL;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);


void callback(char* topic, byte* payload, unsigned int length) {
  byte *cmdPtr = payload;
  byte *valPtr = NULL;
  String top, cmd, val;

  payload[length] = '\0';
  for (int i = 0; i < length; i++) {
    if (payload[i] == '=') {
      cmdPtr[i] = '\0';
      valPtr = &payload[i + 1];
    }
  }
  top = String(topic);
  cmd = String((char *)cmdPtr);
  val = String((char *)valPtr);

  sn.consolePrintln(top + ", " + cmd + ", " + val);

  String msg;
  if (cmd.equals("RSSI")) {
    SensorNet::WIFI_STATE wifiState = sn.wifiState();
    msg = "RSSI=" + String(wifiState.rssi);
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::RESPONSE, msg);
  } else if (cmd.equals("rate")) {
    if (val == NULL) {
      msg = "rate=" + String(REPORT_INTERVAL);
      sn.consolePrintln(msg);
      sn.mqttPub(SensorNet::RESPONSE, msg);
    } else {
      sn.consolePrintln("ERROR: unable to set rate");
    }
  } else if (cmd.equals("version")) {
    msg = "Version=" + String(APP_VERSION);
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::RESPONSE, msg);
  } else if (cmd.equals("reset")) {
    sn.consolePrintln("Resetting");
    sn.systemReset();
  } else {
    msg = "ERROR: unknown command (" + cmd + ")";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
  }
}

void setup() {
  sn.serialStart(&Serial, 9600, false);
  sn.serialStart(&Serial1, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  if (!sn.mqttSub(SensorNet::COMMAND, callback)) {
    sn.consolePrintln("Resetting");
    delay(60000);
    sn.systemReset();
  }
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
