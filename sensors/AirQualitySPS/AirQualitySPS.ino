/*
* Air Quality Sensor: using SPS30 sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include <sps30.h>

#define APP_NAME        "AirQualitySPS"
#define APP_VERSION     "1.2.0"
#define REPORT_SCHEMA   "pm1_0:.2f,pm2_5:.2f,pm4_0:.2f,pm10_0:.2f,nc0_5:.2f,nc1_0:.2f,nc2_5:.2f,nc4_0:.2f,nc10_0:.2f,tps:.2f"

#define TOPIC_PREFIX    "/sensors/AirQuality/SPS"

#define DEF_REPORT_INTERVAL 60000   // one report every minute

#define AUTO_CLEAN_DAYS 4       // auto-clean every four days

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0


boolean wakingUp = false;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);


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
  int16_t ret;
  String msg;

  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);

  sensirion_i2c_init();

  if (sps30_probe() != 0) {
    msg = "ERROR: SPS sensor probe failed";
    sn.consolePrintln(msg);
    delay(60000);
    sn.mqttPub(SensorNet::ERROR, msg);
  }

  ret = sps30_set_fan_auto_cleaning_interval_days(AUTO_CLEAN_DAYS);
  if (ret) {
    msg = "ERROR: failed to set auto-clean interval -- " + String(ret);
    sn.consolePrintln(msg);
    delay(60000);
    sn.mqttPub(SensorNet::ERROR, msg);
  }

  if (sps30_start_measurement() < 0) {
    msg = "ERROR: failed to start measurement";
    sn.consolePrintln(msg);
    delay(60000);
    sn.mqttPub(SensorNet::ERROR, msg);
  }

  sn.consolePrintln("measurements started");
  delay(1000);
}

void loop() {
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t dataReady;
  int16_t ret;
  String msg;
  unsigned long now = millis();
  unsigned long deltaT = now - sn.lastReport;

  sn.mqttRun();

  if ((wakingUp == false) && (deltaT >= (sn.reportInterval / 2))) {
    // start up sensor half an interval before reading it to get steady-state reading
    //// TODO assert REPORT_INTERVAL > 30000
    sn.consolePrintln("Waking up sensor");
    if (sps30_wake_up() != 0) {
      msg = "ERROR: failed to wake up sensor";
      sn.consolePrintln(msg);
      delay(60000);
      sn.mqttPub(SensorNet::ERROR, msg);
    } else {
      wakingUp = true;
    }
  }
  if ((wakingUp == true) && (deltaT >= sn.reportInterval)) {
    sn.consolePrintln("Reading sensor");
    while (1) {
      ret = sps30_read_data_ready(&dataReady);
      if (ret < 0) {
        msg = "ERROR: reading data-ready flag: " + String(ret);
        sn.consolePrintln(msg);
        delay(60000);
        sn.mqttPub(SensorNet::ERROR, msg);
      } else if (!dataReady) {
        msg = "WARNING: data not ready";
        sn.consolePrintln(msg);
        sn.mqttPub(SensorNet::ERROR, msg);
      } else {
        break;
      }
      delay(100); /* retry in 100ms */
    }

    if (sps30_read_measurement(&m) >= 0) {
      // PM1.0, PM2.5, PM4.0, PM10, NC0.5, NC1.0, NC2.5, NC4.0, NC10.0, typicalParticleSize
      msg = String(m.mc_1p0) + "," + String(m.mc_2p5) + "," + String(m.mc_4p0) + "," +
            String(m.mc_10p0)  + "," + String(m.nc_0p5)  + "," + String(m.nc_1p0)  + "," +
            String(m.nc_2p5)  + "," + String(m.nc_4p0) + "," + String(m.nc_10p0)  + "," +
            String(m.typical_particle_size);
      sn.mqttPub(SensorNet::DATA, msg);
      sn.consolePrintln(msg);
      wakingUp = false;
    } else {
      msg = "error reading SPS30 sensor";
      sn.consolePrintln(msg);

    }

    sn.consolePrintln("Sleep until next report");
    if (sps30_sleep() != 0) {
      msg = "ERROR: failed to put sensor to sleep";
      sn.consolePrintln(msg);

    }

    sn.lastReport = now;
  }
}
