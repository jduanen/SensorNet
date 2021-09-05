/*
*
* WaterHeater: WiFi, MQTT-based, temperature logging application
*
* Attach water temperature sensor to the output pipe of the water heater (after the recirculation pump)
*  - the ambient temperature sensor is to sense the temperature around the device
*
*/

//// TODO consider using ADC to monitor Vcc

#include <OneWire.h>
#include <DallasTemperature.h>
#include "SensorNet.h"
#include "wifi.h"

#define APP_NAME        "WaterHeater"
#define APP_VERSION     "1.0.4"
#define REPORT_SCHEMA   "waterDegC:3.2f,ambientDegC:3.2f"

#define ONE_WIRE_BUS  2

// N.B. must correspond to how sensors are wired
#define AMBIENT_TEMP_DEV_NUM  0
#define WATER_TEMP_DEV_NUM    1

#define DEF_TEMPERATURE_PRECISION 12

#define TOPIC_PREFIX    "/sensors/WaterHeater"

#define DEF_REPORT_INTERVAL 60000  // one report every minute

#define MQTT_SERVER     "192.168.166.113"
#define MQTT_PORT       1883

#define VERBOSE         0

#define MAX_CMD_LEN     16
#define MAX_VAL_LEN     16


unsigned long lastReport = 0;
unsigned int reportInterval = DEF_REPORT_INTERVAL;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors;
DeviceAddress waterThermometer, ambientThermometer;


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
  } else if (cmd.equals("precision")) {
    uint8_t precision;
    if (val != NULL) {
      precision = val.toInt();
      if ((precision < 9) || (precision > 12)) {
        sn.consolePrintln("ERROR: Invalid precision value: " + val);
      } else {
        sn.consolePrintln("Set precision to: " + val);
        sensors.setResolution(ambientThermometer, precision);
        sensors.setResolution(waterThermometer, precision);
      }
    }
    uint8_t ambPrec = sensors.getResolution(ambientThermometer);
    uint8_t watPrec = sensors.getResolution(waterThermometer);
    if (ambPrec != watPrec) {
      msg = "ERROR: precision mismatch: " + String(ambPrec) + ", " + String(watPrec);
      sn.consolePrintln(msg);
      sn.mqttPub(SensorNet::ERROR, msg);
    } else {
      precision = watPrec;
    }
    msg = "precision=" + String(precision);
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
  int deviceCount = 0;
  sn.serialStart(&Serial, 9600, true);
  sn.consolePrintln(APP_NAME);

  sn.wifiStart(WLAN_SSID, WLAN_PASS);

  sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX);
  if (!sn.mqttSub(SensorNet::DATA, callback)) {
    sn.consolePrintln("Resetting");
    delay(60000);
    sn.systemReset();
  }

  sn.consolePrintln("Init temp sensors");
  sensors = DallasTemperature(&oneWire);
  sensors.begin();
  deviceCount = sensors.getDeviceCount();
  sn.consolePrintln("Found " + String(deviceCount) + " temp sensors");
  String msg;
  if (deviceCount != 2) {
    msg = "ERROR: Unable to find both temp sensors";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
    delay(60000);
    sn.systemReset();
  }
  if (!sensors.getAddress(ambientThermometer, AMBIENT_TEMP_DEV_NUM) ||
      !sensors.getAddress(waterThermometer, WATER_TEMP_DEV_NUM)) {
    msg = "ERROR: Unable to get both temp sensor addresses";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
    delay(60000);
    sn.systemReset();
  }
  if (!sensors.setResolution(ambientThermometer, DEF_TEMPERATURE_PRECISION) ||
      !sensors.setResolution(waterThermometer, DEF_TEMPERATURE_PRECISION)) {
    msg = "ERROR: Failed to set temperature precision";
    sn.consolePrintln(msg);
    sn.mqttPub(SensorNet::ERROR, msg);
    delay(60000);
    sn.systemReset();
  }
}

void loop() {
  String inMsg;
  String msg;
  float tempC;
  unsigned long now = millis();
  unsigned long deltaT = now - lastReport;

  sn.mqttRun();

  if (deltaT >= reportInterval) {
    sn.consolePrintln("Reading sensors");

    sensors.requestTemperatures();
    tempC = sensors.getTempC(waterThermometer);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg = String(tempC);
    } else {
      msg = "N/A";
    }
    tempC = sensors.getTempC(ambientThermometer);
    if (tempC != DEVICE_DISCONNECTED_C) {
      msg += "," + String(tempC);
    } else {
      msg += ",N/A";
    }

    sn.mqttPub(SensorNet::DATA, msg);
    sn.consolePrintln(msg);
    sn.consolePrintln("Sleep until next report");

    lastReport = now;
  }
}
