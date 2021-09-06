/*
*
* BirdyNumNum: Temperature-calibrated, WiFi, MQTT-based, weight logging application
*
*/

#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME        "BirdyNumNum"
#define APP_VERSION     "1.1.0"
#define REPORT_SCHEMA   "intDegC:3.2f,extDegC:3.2f,volts:4d,grams:4.2f"

#define HX711_CLK     5
#define HX711_DOUT    4
#define ONE_WIRE_BUS  2
#define CALIBRATE_PIN 16

// N.B. must corRESPONSE to how sensors are wired
#define LC_TEMP_DEV_NUM   0
#define BAT_TEMP_DEV_NUM  1

#define TOPIC_PREFIX    "/sensors/BirdyNumNum"

#define DEF_REPORT_INTERVAL 60000  // one report every minute

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define SCALE           2898.41

#define VERBOSE         0


unsigned long lastReport = 0;
unsigned int reportInterval = DEF_REPORT_INTERVAL;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

HX711 scale;


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
  String msg;
  int deviceCount = 0;

  pinMode(CALIBRATE_PIN, INPUT);
  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  if (!sn.mqttSub(SensorNet::COMMAND, callback)) {
    sn.consolePrintln("Resetting");
    delay(60000);
    sn.systemReset();
  }

  sn.consolePrintln("Init temp sensors");
  sensors.begin();
  deviceCount = sensors.getDeviceCount();
  sn.consolePrintln("Found " + String(deviceCount) + " temp sensors");
  if (deviceCount != 2) {
    msg = "ERROR: Unable to find both temp sensors, resetting...";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
    delay(60000);
    sn.systemReset();
  }

  sn.consolePrintln("Init scale");
  scale.begin(HX711_DOUT, HX711_CLK);
  scale.reset();
  if (!scale.wait_ready_retry(100, 100)) {
    msg = "Scale not ready, resetting...";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
    delay(60000);
    sn.systemReset();
  }
  //// TODO switch to median
  scale.tare();
  if (digitalRead(CALIBRATE_PIN) == 0) {
    sn.consolePrintln("Calibrating");
    sn.consolePrintln("Empty scale, then hit enter");
    sn.consoleWaitForInput();
    sn.consolePrintln("Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
    sn.consolePrintln("Put 50g weight on scale, then hit enter");
    sn.consoleWaitForInput();
    scale.calibrate_scale(50, 5);
    sn.consolePrintln("Scale: " + String(scale.get_scale()) + ", Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
  } else {
    scale.set_scale(SCALE);
    sn.consolePrintln("Set Scale=" + String(SCALE) + ", Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
  }
  sn.consolePrintln("Scale calibrated");
}

void loop() {
  String inMsg;
  String msg;
  float tempC;
  int volts;
  float units;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  //// TODO add conditionals to power up/down the scale

  if (deltaT >= reportInterval) {
    sn.consolePrintln("Reading sensors");

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(BAT_TEMP_DEV_NUM);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg = String(tempC);
    } else {
      msg = "N/A";
    }
    tempC = sensors.getTempCByIndex(LC_TEMP_DEV_NUM);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg += "," + String(tempC);
    } else {
      msg += ",N/A";
    }

    volts = analogRead(A0);
    msg += "," + String(volts);

    units = scale.get_units(5);
    msg += "," + String(units);

    sn.mqttPub(SensorNet::DATA, msg);
    sn.consolePrintln(msg);

    sn.consolePrintln("Sleep until next report");

    lastReport = now;
  }
}
