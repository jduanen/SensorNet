#include "SensorNet.h"
#include "wifi.h"

#define REPORT_INTERVAL 60000  // 60 secs

#define MQTT_SERVER   "192.168.166.113"
#define MQTT_PORT     1883

unsigned long lastReport = 0;
String msg;
int i = 0;

SensorNet sn("MQTT_TEST");

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.println("Message arrived in topic: " + String(topic));

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
}

void callbackX(char *, byte *, unsigned int) {
  Serial.println("MMMMM");
}

void setup() {
  sn.serialStart(&Serial, 9600, true);
  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, "/sensors/test");
  sn.mqttSub(callback);
}

void loop() {
  sn.mqttRun();

  unsigned long now = millis();
  if ((now - lastReport) > REPORT_INTERVAL) {
    msg = "testing" + String(i++);
    sn.mqttPub(msg);
    Serial.println("Pub: " + msg);
    lastReport = now;
  }
}
