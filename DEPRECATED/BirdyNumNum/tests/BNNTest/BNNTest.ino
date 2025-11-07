/*
* Test program for main units of BirdyNomNom
*/

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS                6    // GPIO6 aka D6

// N.B. must correspond to how sensors are wired
#define LOADCELL_TEMP_DEV_NUM       0
#define AMBIENT_TEMP_DEV_NUM        1

#define DEF_TEMPERATURE_PRECISION   12

#define NUM_CALIBRATION_STEPS       3


uint8_t tempPrecision = DEF_TEMPERATURE_PRECISION;

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors;
DeviceAddress loadcellThermometer, ambientThermometer, tempDeviceAddress;


void printDeviceAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; (i < 8); i++) {
        if (deviceAddress[i] < 16) {
            Serial.print("0");
        } else {
            Serial.print(deviceAddress[i], HEX);
        }
    }
};

void setup(void) {
    int deviceCount = 0;
    Serial.begin(115200);
    delay(500);
    Serial.println("BEGIN");

    Serial.println("PIN: " + String(ONE_WIRE_BUS)); //// TMP TMP TMP
  
    Serial.println("Init temp sensors");
    sensors = DallasTemperature(&oneWire);
    sensors.begin();
    deviceCount = sensors.getDeviceCount();
    Serial.println("Found " + String(deviceCount) + " temp sensors");

    if (deviceCount != 2) {
        Serial.println("ERROR: Unable to find both temp sensors");
        delay(60000);
        //// restart
    }
    if (!sensors.getAddress(ambientThermometer, AMBIENT_TEMP_DEV_NUM) ||
        !sensors.getAddress(loadcellThermometer, LOADCELL_TEMP_DEV_NUM)) {
        Serial.println("ERROR: Unable to get both temp sensor addresses");
        delay(60000);
        //// restart
    }
    if (!sensors.setResolution(ambientThermometer, tempPrecision) ||
        !sensors.setResolution(loadcellThermometer, tempPrecision)) {
        Serial.println("ERROR: Failed to set temperature precision");
        delay(60000);
        //// restart
    }

    Serial.print("Ambient Thermometer Address: ");
    printDeviceAddress(ambientThermometer);
    Serial.println("");

    Serial.print("Loadcell Thermometer Address: ");
    printDeviceAddress(loadcellThermometer);
    Serial.println("");

    // read and discard first values from both sensors
    (void)sensors.getTempC(loadcellThermometer);
    (void)sensors.getTempC(ambientThermometer);
    Serial.println("READY");
}

void loop(void) {
    float tempC;
    sensors.requestTemperatures();

    tempC = sensors.getTempC(loadcellThermometer);
    Serial.println("Loadcell Temp: " + String(tempC, 3) + " C");

    tempC = sensors.getTempC(ambientThermometer);
    Serial.println("Ambient Temp:  " + String(tempC, 3) + " C");

    Serial.println("");
    delay(1000);
}
