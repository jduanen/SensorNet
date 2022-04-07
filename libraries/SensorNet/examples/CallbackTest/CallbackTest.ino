/***************************************************************************
 *
 * SensorNet Arduino Library Callback test
 *
 * Receives standard messages and generates response messages
 * 
 * Notes
 *   - listener
 *     * mosquitto_sub -h localhost -t "/test/callback/#" -F "%I,%t,%p"
 *   - example tests
 *     * issue get 'cmd' to a specific device
 *       - mosquitto_pub -t "/test/callback/typeA/<ipAddr>/cmd" -m "<cmd>"
 *     * issue set 'cmd' to 'val' on a specific device
 *       - mosquitto_pub -t "/test/callback/typeA/<ipAddr>/cmd" -m "<cmd>=<val>"
 *     * issue get 'cmd' to all devices of type '/test/callback/typeA'
 *       - mosquitto_pub -t "/test/callback/typeA/cmd" -m "<cmd>"
 *
 ***************************************************************************/

#include <Arduino.h>
#include "SensorNet.h"
#include "wifi.h"


#define VERBOSE             1
#define APP_NAME            "CallbackTest"
#define APP_VERSION         "1.0.0"
#define REPORT_SCHEMA       "intVar:d,strVar:s,rssi:d"
#define TOPIC_PREFIX        "/test/callback/typeA"
#define MQTT_SERVER         "192.168.166.113"
#define MQTT_PORT           1883


int intVar = 0;
String strVar = "";

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

// Callback function header
callback myCallback;

void print(String s) {
  if (VERBOSE) {
    sn.consolePrint(s);
  }
}

void println(String s) {
  if (VERBOSE) {
    sn.consolePrintln(s);
  }
}

void myCallback(char* topic, byte* payload, unsigned int length) {
  String respMsg;
  char msgType = SensorNet::RESPONSE;
  println("myCallback");
  SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);

  if (cbMsg.handled == true) {
    println("Callback message handled by baseCallback");
    // N.B. you can do other stuff in addition to what's done in the base hander here
  } else {
    if (cbMsg.cmd.equalsIgnoreCase("intVar")) {
      if (cbMsg.val != NULL) {
        intVar = cbMsg.val.toInt();
        println("intVar = " + String(intVar));
      }
      respMsg = "intVar=" + String(intVar);
    } else if (cbMsg.cmd.equalsIgnoreCase("strVar")) {
      if (cbMsg.val != NULL) {
        println("strVar = " + cbMsg.val);
        strVar = cbMsg.val;
      }
      respMsg = "strVar=" + strVar;
    } else {
      respMsg = "ERROR: unknown command (" + cbMsg.cmd + ")";
      msgType = SensorNet::ERROR;
    }
    println("Response Message: " + respMsg);
    sn.mqttPub(msgType, respMsg);
  }
}

void setup() {
  sn.serialStart(&Serial, 9600, true);
  delay(500);
  println(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);
  println("WIFI");

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);
  println("MQTT");
}

#define WAIT_TIME 10000

unsigned int indx = 0;

#define NUM_STRS  10

void loop() {
  String msg;
  unsigned long now = millis();
  unsigned long deltaT = now - sn.lastReport;

  sn.mqttRun();

  if (deltaT >= sn.reportInterval) {
    msg = String(intVar) + "," + strVar + "," + String(sn.wifiState().rssi);

    sn.mqttPub(SensorNet::DATA, msg);
    println("Status Msg: " + msg);
    sn.lastReport = now;
  }
};
