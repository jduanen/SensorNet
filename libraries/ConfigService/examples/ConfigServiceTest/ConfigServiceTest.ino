/***************************************************************************
 *
 * Configuration Service Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"
#include "LfsUtilities.h"


#define VERBOSE         1
#define APP_NAME        "ConfigServiceTest"
#define APP_VERSION     "1.0.0"
#define CONFIG_PATH     "/config.json"


typedef struct {
    String  str;
    int     i;
    char    a;
    bool    b;
    float   c;
    char    d[];
} ConfigState;

ConfigState configState = {
    String("Test String"),
    1234,
    'a',
    true,
    3.14,
    "char array"
};


void halt() {
    while (true) {};
}

//// TODO move these to subclass of ConfigService and call them as methods
void setConfig(ConfigService *csPtr, ConfigState *structPtr) {
    csPtr->configJsonDoc["str"] = structPtr->str;
    csPtr->configJsonDoc["i"] = structPtr->i;
    csPtr->configJsonDoc["a"] = structPtr->a;
    csPtr->configJsonDoc["b"] = structPtr->b;
    csPtr->configJsonDoc["c"] = structPtr->c;
}

void getConfig(ConfigService *csPtr, ConfigState *structPtr) {
//    structPtr->str = csPtr->configJsonDoc["str"];
    structPtr->i = csPtr->configJsonDoc["i"];
    structPtr->a = csPtr->configJsonDoc["a"];
    structPtr->b = csPtr->configJsonDoc["b"];
    structPtr->c = csPtr->configJsonDoc["c"];
}

void setup() {
    delay(500);
    Serial.begin(19200);
    delay(500);
    Serial.print("\nBEGIN: ");
    Serial.println(APP_NAME);
    Serial.println("==========================");

    if (true) {
        Serial.println("Formatting LittleFS");
        formatLFS();
    }
    listDir("/");
    Serial.println("==========================");

    if (true) {
        // should create an empty config file as none exists
        ConfigService cs = ConfigService(CONFIG_PATH);
        Serial.println("1vvvvvvvvvvvvvvvvvvvvvvvvvv1");
        listDir("/");
        Serial.println("Contents of config file:");
        cs.printConfig();
        Serial.println("");
        Serial.println("1^^^^^^^^^^^^^^^^^^^^^^^^^1");
    }

    if (true) {
        ConfigService cs = ConfigService(CONFIG_PATH);
        Serial.println("2vvvvvvvvvvvvvvvvvvvvvvvvvv2");
        Serial.println("Initialize the config");
        setConfig(&cs, &configState);
        cs.saveConfig();
        listDir("/");
        Serial.println("Contents of initialized config file:");
        cs.printConfig();
        Serial.println("");
        Serial.println("2^^^^^^^^^^^^^^^^^^^^^^^^^2");
    }

    if (true) {
        ConfigService cs = ConfigService(CONFIG_PATH);
        Serial.println("3vvvvvvvvvvvvvvvvvvvvvvvvvv3");
        Serial.println("Update the config");
        getConfig(&cs, &configState);
        configState.i++;
        configState.b = !configState.b;
        setConfig(&cs, &configState);
        Serial.println("current config:");
        cs.saveConfig();
        listDir("/");
        Serial.println("current config:");
        cs.printConfig();
        Serial.println("3^^^^^^^^^^^^^^^^^^^^^^^^^3");
    }

    if (true) {
        // read the new config file
        Serial.print("After update: ");
        ConfigService cs = ConfigService(CONFIG_PATH);
        listDir("/");
        cs.printConfig();
        Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
}

void loop() {
    delay(1000);
};
