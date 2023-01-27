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


#define VERBOSE             1

#define APP_NAME            "BirdyNumNum"
#define APP_VERSION         "1.3.1"
#define REPORT_SCHEMA       "intDegC:3.2f,extDegC:3.2f,volts:4d,grams:4.2f"

#define HX711_CLK           5
#define HX711_DOUT          4
#define ONE_WIRE_BUS        2
#define CALIBRATE_PIN       16
#define BATTERY_PIN         A0

// N.B. must correspond to how sensors are wired
#define LC_TEMP_DEV_NUM     0
#define BAT_TEMP_DEV_NUM    1

#define DEF_TEMPERATURE_PRECISION 12

#define TOPIC_PREFIX        "/sensors/BirdyNumNum"

#define DEF_REPORT_INTERVAL 60000  // one report every minute

#define MQTT_SERVER         "192.168.166.113"
#define MQTT_PORT           1883

#define DEF_SCALE_VAL       2898.41

#define COMMAND_NAMES       "Calibrate,Gain,Offset,Precision,Scale,ScalePower,Tare,Units,Volts"


typedef struct ScaleInfoStr {
    float           units;
    float           tare;
    unsigned long   offset;
    float           scale;
    uint8_t         gain;
} ScaleInfo;


bool scalePower = false;

unsigned int reportInterval = DEF_REPORT_INTERVAL;

SensorNet sn(APP_NAME, APP_VERSION, REPORT_SCHEMA, COMMAND_NAMES);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress loadcellThermometer, batteryThermometer;

HX711 scale;


void printScaleInfo(ScaleInfo *siPtr) {
    sn.consolePrintln("Units: " + String(siPtr->units) + \
                      ", Tare: " + String(siPtr->tare) + \
                      ", Offset: " + String(siPtr->offset) + \
                      ", Scale: " + String(siPtr->scale) + \
                      ", Gain: " + String(siPtr->gain));
}

void getScaleInfo(ScaleInfo *siPtr) {
    siPtr->units = scale.get_units(10);
    siPtr->tare = scale.get_tare();
    siPtr->offset = scale.get_offset();
    siPtr->scale = scale.get_scale();
    siPtr->gain = scale.get_gain();
}

void myCallback(char* topic, byte* payload, unsigned int length) {
    ScaleInfo si;
    SensorNet::callbackMessage cbMsg = sn.baseCallback(topic, payload, length);

    if (cbMsg.handled == true) {
        sn.consolePrintln("Callback message handled by baseCallback");
    } else {
        String respMsg;
        char msgType = SensorNet::RESPONSE;

        if (cbMsg.cmd.equalsIgnoreCase("Calibrate")) {
            if (cbMsg.val == NULL) {
                sn.consolePrintln("Tare to calibrate to zero");
                scale.tare();
            } else {
                uint16_t weight = cbMsg.val.toInt();
                sn.consolePrintln("Calibrating with " + String(weight) + " grams...");
                scale.calibrate_scale(weight, 5);
            }
            getScaleInfo(&si);
            printScaleInfo(&si);
            respMsg = "Units=" + String(si.units) + \
                      ",Tare=" + String(si.tare) + \
                      ",Offset=" + String(si.offset) + \
                      ",Scale=" + String(si.scale) + \
                      ",Gain=" + String(si.gain);
        } else if (cbMsg.cmd.equalsIgnoreCase("Gain")) {
            uint8_t gain = scale.get_gain();
            respMsg = "Gain=" + String(gain);
        } else if (cbMsg.cmd.equalsIgnoreCase("Offset")) {
            if (cbMsg.val != NULL) {
                scale.set_scale(cbMsg.val.toInt());
            }
            unsigned long offset = scale.get_offset();
            respMsg = "Offset=" + String(offset);
        } else if (cbMsg.cmd.equalsIgnoreCase("Precision")) {
            uint8_t precision;
            if (cbMsg.val != NULL) {
                precision = cbMsg.val.toInt();
                if ((precision < 9) || (precision > 12)) {
                    sn.consolePrintln("WARNING: Invalid precision value: " + cbMsg.val);
                } else {
                    sn.consolePrintln("Set precision to: " + cbMsg.val);
                    sensors.setResolution(loadcellThermometer, precision);
                    sensors.setResolution(batteryThermometer, precision);
                }
            }
            uint8_t lcPrec = sensors.getResolution(loadcellThermometer);
            uint8_t batPrec = sensors.getResolution(batteryThermometer);
            if (lcPrec != batPrec) {
                respMsg = "ERROR: precision mismatch: " + String(lcPrec) + ", " + String(batPrec);
                msgType = SensorNet::ERROR;
            } else {
                precision = batPrec;
                respMsg = "Precision=" + String(precision);
            }
        } else if (cbMsg.cmd.equalsIgnoreCase("Scale")) {
            if (cbMsg.val != NULL) {
                scale.set_scale(cbMsg.val.toFloat());
            }
            float s = scale.get_scale();
            respMsg = "Scale=" + String(s);
        } else if (cbMsg.cmd.equalsIgnoreCase("ScalePower")) {
            if (cbMsg.val != NULL) {
                if (cbMsg.val.equalsIgnoreCase("on")) {
                    scale.power_up();
                    scalePower = true;
                } else if (cbMsg.val.equalsIgnoreCase("off")) {
                    scale.power_down();
                    scalePower = false;
                } else {
                    sn.consolePrintln("WARNING: ignoring invalid value (" + cbMsg.val + ")");
                }
            }
            respMsg = "ScalePower=";
            if (scalePower) {
                respMsg += "ON";
            } else {
                respMsg += "OFF";
            }
        } else if (cbMsg.cmd.equalsIgnoreCase("Tare")) {
            if (cbMsg.val != NULL) {
                sn.consolePrintln("WARNING: ignoring value");
            }
            float tare = scale.get_tare();
            respMsg = "Tare=" + String(tare);
        } else if (cbMsg.cmd.equalsIgnoreCase("Units")) {
            if (cbMsg.val != NULL) {
                sn.consolePrintln("WARNING: ignoring value");
            }
            float units = scale.get_units();
            respMsg = "Units=" + String(units);
        } else if (cbMsg.cmd.equalsIgnoreCase("Volts")) {
            if (cbMsg.val != NULL) {
                sn.consolePrintln("WARNING: ignoring value");
            }
            respMsg = "Volts=" + String(analogRead(BATTERY_PIN));
        } else {
            respMsg = "ERROR: unhandled command: " + cbMsg.cmd;
            msgType = SensorNet::ERROR;
        }
        sn.consolePrintln("Response Message: " + respMsg);
        sn.mqttPub(msgType, respMsg);
    }
}

void setup() {
    String  msg;
    int deviceCount = 0;

    pinMode(CALIBRATE_PIN, INPUT);
    sn.serialStart(&Serial, 19200, true);
    sn.consolePrintln(APP_NAME);

    sn.wifiStart(WLAN_SSID, WLAN_PASS);

    sn.mqttSetup(MQTT_SERVER, MQTT_PORT, TOPIC_PREFIX, myCallback);

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

    if (!sensors.getAddress(loadcellThermometer, LC_TEMP_DEV_NUM) ||
        !sensors.getAddress(batteryThermometer, BAT_TEMP_DEV_NUM)) {
        msg = "ERROR: Unable to get both temp sensor addresses";
        sn.consolePrintln(msg);
        sn.mqttPub(SensorNet::ERROR, msg);
        delay(60000);
        sn.systemReset();
    }
    if (!sensors.setResolution(loadcellThermometer, DEF_TEMPERATURE_PRECISION) ||
        !sensors.setResolution(batteryThermometer, DEF_TEMPERATURE_PRECISION)) {
        msg = "ERROR: Failed to set temperature precision";
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
    scale.power_up();
    scalePower = true;

    //// TODO switch to median
    if (digitalRead(CALIBRATE_PIN) == 0) {
        ScaleInfo si;

        sn.consolePrintln("Calibrating...");
        sn.consolePrintln("Empty scale, then hit enter");
        sn.consoleWaitForInput();

        scale.tare();
        sn.consolePrintln("Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));

        sn.consolePrintln("Put 50g weight on scale, then hit enter");
        sn.consoleWaitForInput();

        scale.calibrate_scale(50, 5);

        getScaleInfo(&si);
        printScaleInfo(&si);
    } else {
        scale.tare();
        scale.set_scale(DEF_SCALE_VAL);
        sn.consolePrintln("Set Scale=" + String(DEF_SCALE_VAL) + ", Units: " + String(scale.get_units(10)) + ", Tare: " + String(scale.get_tare()));
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
    unsigned long deltaT = now - sn.lastReport;

    sn.mqttRun();

    if (deltaT >= sn.reportInterval) {
        sn.consolePrintln("Reading sensors");

        sensors.requestTemperatures();
        tempC = sensors.getTempC(batteryThermometer);
        if (tempC != DEVICE_DISCONNECTED_C) {
            msg = String(tempC);
        } else {
            msg = "N/A";
        }
        tempC = sensors.getTempC(loadcellThermometer);
        if (tempC != DEVICE_DISCONNECTED_C) {
            msg += "," + String(tempC);
        } else {
            msg += ",N/A";
        }

        volts = analogRead(BATTERY_PIN);
        msg += "," + String(volts);

        units = scale.get_units(5);
        msg += "," + String(units);

        sn.mqttPub(SensorNet::DATA, msg);
        sn.consolePrintln(msg);
        sn.consolePrintln("Sleep until next report");

        sn.lastReport = now;
    }
}
