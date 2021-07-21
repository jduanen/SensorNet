/*
* Air Quality Sensor: using SPS30 sensor
*/

#include "SensorNet.h"
#include "wifi.h"
#include <sps30.h>

#define APP_NAME        "AirQualitySPS"

#define TOPIC_PREFIX    "/sensors/AirQuality/SPS"

#define REPORT_INTERVAL 60000   // one report every 60 secs

#define AUTO_CLEAN_DAYS 4       // auto-clean every four days

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0

SensorNet sn(APP_NAME);

void callback(char* topic, byte* payload, unsigned int length) {
  sn.consolePrintln("Message arrived in topic: " + String(topic));

  sn.consolePrint("Message: ");
  for (int i = 0; i < length; i++) {
    sn.consolePrint(String(payload[i]));
  }
  sn.consolePrintln("");
}

void setup() {
  int16_t ret;

  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  ////sn.mqttSub(callback);

  sensirion_i2c_init();

  while (sps30_probe() != 0) {
    sn.consolePrintln("ERROR: SPS sensor probe failed");
    delay(500);
  }

  ret = sps30_set_fan_auto_cleaning_interval_days(AUTO_CLEAN_DAYS);
  if (ret) {
    sn.consolePrintln("ERROR: failed to set auto-clean interval -- " + String(ret));
  }

  if (sps30_start_measurement() < 0) {
    sn.consolePrintln("ERROR: failed to start measurement");
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

  sn.mqttRun();

  while (1) {
    ret = sps30_read_data_ready(&dataReady);
    if (ret < 0) {
      sn.consolePrintln("ERROR: reading data-ready flag: " + String(ret));
    } else if (!dataReady)
      sn.consolePrintln("data not ready");
    else
      break;
    delay(100); /* retry in 100ms */
  }

  if (sps30_read_measurement(&m) < 0) {
    sn.consolePrintln("error reading SPS30 sensor");
  } else {
    // PM1.0, PM2.5, PM4.0, PM10, NC0.5, NC1.0, NC2.5, NC4.0, NC10.0, typicalParticleSize
    msg = String(m.mc_1p0) + "," + String(m.mc_2p5) + "," + String(m.mc_4p0) + "," +
          String(m.mc_10p0)  + "," + String(m.nc_0p5)  + "," + String(m.nc_1p0)  + "," +
          String(m.nc_2p5)  + "," + String(m.nc_4p0) + "," + String(m.nc_10p0)  + "," +
          String(m.typical_particle_size);
    sn.mqttPub(msg);
    sn.consolePrintln(msg);
  }

  delay(REPORT_INTERVAL);
}
