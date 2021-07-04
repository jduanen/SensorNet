#include "SensorNet.h"
#include "wifi.h"

#define MQTT_SERVER   "192.168.166.113"
#define MQTT_PORT     1883

String msg;
int i = 0;

SensorNet sn;

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.println("Message arrived in topic: " + String(topic));

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
}

void setup() {
  sn.serialStart(&Serial, 9600, true);
  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttStart(MQTT_SERVER, MQTT_PORT, "/sensors/test");
  sn.mqttSub(callback);
}

void loop() {
  SensorNet::WIFI_STATE state = sn.wifiState();
  Serial.println(state.macAddr);

  msg = "testing" + String(i++);
  sn.mqttPub(msg);
  delay(3000);
}
