/*
* Air Quality Sensor: using PMS7003 sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include "PMS.h"

#define APP_NAME        "AirQualityPMS"
#define APP_VERSION     "1.3.0"
#define REPORT_SCHEMA   "pm1_0:d,pm2_5:d,pm10_0:d"

#define TOPIC_PREFIX    "/sensors/AirQuality/PMS"

#define DEF_REPORT_INTERVAL 60000  // one report every minute

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define PMS_SLEEP_PIN   5  // pin GPIO5/D1: low=sleep, high=run
#define PMS_RESET_PIN   4  // pin GPIO4/D2: low=reset, high=run

#define VERBOSE         0


boolean wakingUp = false;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

PMS pms(Serial);
PMS::DATA data;


void myCallback(char* topic, byte* payload, unsigned int length) {
  SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);
  if (cbMsg.handled == true) {
    sn.consolePrintln("Callback message handled by baseCallback");
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

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);

  // start sensor in run mode
  pinMode(PMS_SLEEP_PIN, OUTPUT);
  digitalWrite(PMS_SLEEP_PIN, HIGH);
  pinMode(PMS_RESET_PIN, OUTPUT);
  digitalWrite(PMS_RESET_PIN, HIGH);

  pms.passiveMode();    // Switch to passive mode
}

void loop() {
  String msg;
  unsigned long now = millis();
  unsigned long deltaT = now - sn.lastReport;

  sn.mqttRun();

  if ((wakingUp == false) && (deltaT >= (sn.reportInterval / 2))) {
    // start up sensor half an interval before reading it to get steady-state reading
    //// TODO assert REPORT_INTERVAL > 30000
    sn.consolePrintln("Waking up sensor");
    pms.wakeUp();
    wakingUp = true;
  }
  if ((wakingUp == true) && (deltaT >= sn.reportInterval)) {
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
      sn.mqttPub(SensorNet::DATA, msg);
      sn.consolePrintln(msg);
      wakingUp = false;
    } else {
      msg = "No data from sensor, resetting";
      sn.consolePrintln(msg);
      sn.mqttPub(SensorNet::ERROR, msg);
      delay(60000);
      sn.systemReset();
    }

    sn.consolePrintln("Sleep until next report");
    pms.sleep();

    sn.lastReport = now;
  }
}
