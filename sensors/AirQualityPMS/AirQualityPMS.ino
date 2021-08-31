/*
* Air Quality Sensor: using PMS7003 sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include "PMS.h"

#define APP_NAME        "AirQualityPMS"
#define APP_VERSION     "1.0.0"
#define REPORT_SCHEMA   "pm1_0:d,pm2_5:d,pm10_0:d"

#define TOPIC_PREFIX    "/sensors/AirQuality/PMS"

#define REPORT_INTERVAL 60000  // one report every 60 secs

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0

unsigned long lastReport = 0;
boolean wakingUp = false;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

PMS pms(Serial);
PMS::DATA data;

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

  pms.passiveMode();    // Switch to passive mode
}

void loop() {
  String msg;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  if ((wakingUp == false) && (deltaT >= (REPORT_INTERVAL - 30000))) {
    // start up sensor 30 seconds before reading it to get steady-state reading
    //// TODO assert REPORT_INTERVAL > 30000
    sn.consolePrintln("Waking up sensor");
    pms.wakeUp();
    wakingUp = true;
  }
  if ((wakingUp == true) && (deltaT >= REPORT_INTERVAL)) {
    sn.consolePrintln("Reading sensor");
    pms.requestRead();

    // wait less than one sec for read to complete
    if (pms.readUntil(data)) {
      if (VERBOSE) {
        sn.consolePrintln("PM 1.0 (ug/m3): " + String(data.PM_AE_UG_1_0));
        sn.consolePrintln("PM 2.5 (ug/m3): " + String(data.PM_AE_UG_2_5));
        sn.consolePrintln("PM 10.0 (ug/m3): " + String(data.PM_AE_UG_10_0));
      }

      msg = String(data.PM_AE_UG_1_0) + "," + 
            String(data.PM_AE_UG_2_5) + "," + 
            String(data.PM_AE_UG_10_0);
      sn.mqttPub(msg);
      sn.consolePrintln(msg);
      wakingUp = false;
    } else {
      sn.consolePrintln("No data from sensor, restarting");
      pms = PMS(Serial);
      pms.passiveMode();
      return;
    }

    sn.consolePrintln("Sleep until next report");
    pms.sleep();

    lastReport = now;
  }
}
