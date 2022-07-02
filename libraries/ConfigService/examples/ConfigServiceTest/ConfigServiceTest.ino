/***************************************************************************
 *
 * Configuration Service Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"
#include "LfsUtilities.h"


#ifndef VERBOSE
#define VERBOSE         1
#endif
#define APP_NAME        "ConfigServiceTest"
#define APP_VERSION     "1.0.0"
#define CONFIG_PATH     "/config.json"



typedef struct {
    int     i;
    float   c;
} ConfigSubState;

ConfigSubState configSubState = {
    -1,
    1.0
};

//// TODO include examples of using arrays
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

//// TODO move this to subclass of ConfigService and call them as methods
// Use values from local struct if corresponding value not found in config doc
void initConfig(ConfigService *csPtr, ConfigState *structPtr) {
    if (!csPtr->configJsonDoc.containsKey("str")) {
        csPtr->configJsonDoc["str"] = structPtr->str;
    }
    if (!csPtr->configJsonDoc.containsKey("i")) {
        csPtr->configJsonDoc["i"] = structPtr->i;
    }
    if (!csPtr->configJsonDoc.containsKey("a")) {
        csPtr->configJsonDoc["a"] = structPtr->a;
    }
    if (!csPtr->configJsonDoc.containsKey("b")) {
        csPtr->configJsonDoc["b"] = structPtr->b;
    }
    if (!csPtr->configJsonDoc.containsKey("c")) {
        csPtr->configJsonDoc["c"] = structPtr->c;
    }
}

//// TODO move this to subclass of ConfigService and call them as methods
// Load config doc with values from local struct
void setConfig(ConfigService *csPtr, ConfigState *structPtr) {
    csPtr->configJsonDoc["str"] = structPtr->str;
    csPtr->configJsonDoc["i"] = structPtr->i;
    csPtr->configJsonDoc["a"] = structPtr->a;
    csPtr->configJsonDoc["b"] = structPtr->b;
    csPtr->configJsonDoc["c"] = structPtr->c;
}

//// TODO move this to subclass of ConfigService and call them as methods
// Load local struct with values from config doc
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
        Serial.println("1vvvvvvvvvvvvvvvvvvvvvvvvvv1");
        ConfigService cs = ConfigService(CONFIG_PATH);
        listDir("/");
    }

    if (true) {
        // initialize the config file with a subset of the fields
        Serial.println("2vvvvvvvvvvvvvvvvvvvvvvvvvv2");
        ConfigService cs = ConfigService(CONFIG_PATH);
        Serial.println("Contents of config file:");
        cs.printConfig();
        Serial.println("Initialize subset of the config");
        cs.configJsonDoc["i"] = configSubState.i;
        cs.configJsonDoc["c"] = configSubState.c;
        cs.saveConfig();
        listDir("/");
    }

    if (true) {
        Serial.println("3vvvvvvvvvvvvvvvvvvvvvvvvvv3");
        ConfigService cs = ConfigService(CONFIG_PATH);
        listDir("/");
        Serial.print("Contents of config file: ");
        cs.printConfig();
        Serial.println("Initialize with the full config");
        initConfig(&cs, &configState);
        cs.saveConfig();
        listDir("/");
    }

    if (true) {
        Serial.println("4vvvvvvvvvvvvvvvvvvvvvvvvvv4");
        ConfigService cs = ConfigService(CONFIG_PATH);
        listDir("/");
        Serial.print("Contents of config file: ");
        cs.printConfig();
        Serial.println("Initialize with the full config");
        setConfig(&cs, &configState);
        cs.saveConfig();
        listDir("/");
    }

    if (true) {
        Serial.println("5vvvvvvvvvvvvvvvvvvvvvvvvvv5");
        ConfigService cs = ConfigService(CONFIG_PATH);
        listDir("/");
        Serial.print("Contents of complete config file: ");
        cs.printConfig();
        Serial.println("Update the config");
        getConfig(&cs, &configState);
        configState.i++;
        configState.b = !configState.b;
        setConfig(&cs, &configState);
        cs.saveConfig();
        listDir("/");
    }

    if (true) {
        // read the new config file
        ConfigService cs = ConfigService(CONFIG_PATH);
        Serial.print("Config file after update: ");
        cs.printConfig();
        Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
}

void loop() {
    delay(1000);
};
