/*
* Air Quality Sensor: using PMS7003 sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include "PMS.h"

#define APP_NAME        "AirQualityPMS"
#define APP_VERSION     "1.1.0"
#define REPORT_SCHEMA   "pm1_0:d,pm2_5:d,pm10_0:d"

#define TOPIC_PREFIX    "/sensors/AirQuality/PMS"

#define DEF_REPORT_INTERVAL 60000  // one report every minute

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0


unsigned long lastReport = 0;
unsigned int reportInterval = DEF_REPORT_INTERVAL;
boolean wakingUp = false;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

PMS pms(Serial);
PMS::DATA data;


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
    if (val != NULL) {
      sn.consolePrintln("Set rate to " + val);
      reportInterval = val.toInt();
    }
    msg = "rate=" + String(reportInterval);
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::RESPONSE, msg);
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

  pms.passiveMode();    // Switch to passive mode
}

void loop() {
  String msg;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  if ((wakingUp == false) && (deltaT >= (reportInterval / 2))) {
    // start up sensor half an interval before reading it to get steady-state reading
    //// TODO assert REPORT_INTERVAL > 30000
    sn.consolePrintln("Waking up sensor");
    pms.wakeUp();
    wakingUp = true;
  }
  if ((wakingUp == true) && (deltaT >= reportInterval)) {
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

    lastReport = now;
  }
}
