#include "SensorNet.h"
#include "wifi.h"

SensorNet sn;

void setup() {
  sn.serialStart(&Serial, 9600, true);
  sn.wifiStart(WLAN_SSID, WLAN_PASS);
}

void loop() {
  SensorNet::WIFI_STATE state = sn.wifiState();
  Serial.println(state.macAddr + ", " + state.ipAddr.toString());
  delay(5000);
}
