/***************************************************************************
 *
 * Configuration Service Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"
#include "LfsUtilities.h"


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
void initConfig(ConfigState *structPtr) {
    if (!cs.configJsonDoc.containsKey("str")) {
        cs.configJsonDoc["str"] = structPtr->str;
    }
    if (!cs.configJsonDoc.containsKey("i")) {
        cs.configJsonDoc["i"] = structPtr->i;
    }
    if (!cs.configJsonDoc.containsKey("a")) {
        cs.configJsonDoc["a"] = structPtr->a;
    }
    if (!cs.configJsonDoc.containsKey("b")) {
        cs.configJsonDoc["b"] = structPtr->b;
    }
    if (!cs.configJsonDoc.containsKey("c")) {
        cs.configJsonDoc["c"] = structPtr->c;
    }
}

//// TODO move this to subclass of ConfigService and call them as methods
// Load config doc with values from local struct
void setConfig(ConfigState *structPtr) {
    cs.configJsonDoc["str"] = structPtr->str;
    cs.configJsonDoc["i"] = structPtr->i;
    cs.configJsonDoc["a"] = structPtr->a;
    cs.configJsonDoc["b"] = structPtr->b;
    cs.configJsonDoc["c"] = structPtr->c;
}

//// TODO move this to subclass of ConfigService and call them as methods
// Load local struct with values from config doc
void getConfig(ConfigState *structPtr) {
//    structPtr->str = csPtr->configJsonDoc["str"];
    structPtr->i = cs.configJsonDoc["i"];
    structPtr->a = cs.configJsonDoc["a"];
    structPtr->b = cs.configJsonDoc["b"];
    structPtr->c = cs.configJsonDoc["c"];
}

void setup() {
    delay(500);
    Serial.begin(19200);
    delay(500);
    Serial.print("\nBEGIN: ");
    Serial.println(APP_NAME);
    Serial.println("==========================");

    if (false) {
        Serial.println("Formatting LittleFS");
        lfs.formatLFS();
    }
    lfs.listFilesLong("/");
    Serial.println("==========================");

    if (true) {
        // should create an empty config file as none exists
        Serial.println("1vvvvvvvvvvvvvvvvvvvvvvvvvv1");
        cs.open(CONFIG_PATH);
        lfs.listFilesLong("/");
        cs.close();
    }

    if (true) {
        // initialize the config file with a subset of the fields
        Serial.println("2vvvvvvvvvvvvvvvvvvvvvvvvvv2");
        cs.open(CONFIG_PATH);
        Serial.println("Contents of config file:");
        cs.printConfig();
        Serial.println("Initialize subset of the config");
        cs.configJsonDoc["i"] = configSubState.i;
        cs.configJsonDoc["c"] = configSubState.c;
        cs.saveConfig();
        lfs.listFilesLong("/");
        cs.close();
    }

    if (true) {
        Serial.println("3vvvvvvvvvvvvvvvvvvvvvvvvvv3");
        cs.open(CONFIG_PATH);
        lfs.listFilesLong("/");
        Serial.print("Contents of config file: ");
        cs.printConfig();
        Serial.println("Initialize with the full config");
        initConfig(&configState);
        cs.saveConfig();
        lfs.listFilesLong("/");
        cs.close();
    }

    if (true) {
        Serial.println("4vvvvvvvvvvvvvvvvvvvvvvvvvv4");
        cs.open(CONFIG_PATH);
        lfs.listFilesLong("/");
        Serial.print("Contents of config file: ");
        cs.printConfig();
        Serial.println("Initialize with the full config");
        setConfig(&configState);
        cs.saveConfig();
        lfs.listFilesLong("/");
        cs.close();
    }

    if (true) {
        Serial.println("5vvvvvvvvvvvvvvvvvvvvvvvvvv5");
        cs.open(CONFIG_PATH);
        lfs.listFilesLong("/");
        Serial.print("Contents of complete config file: ");
        cs.printConfig();
        Serial.println("Update the config");
        getConfig(&configState);
        configState.i++;
        configState.b = !configState.b;
        setConfig(&configState);
        cs.saveConfig();
        lfs.listFilesLong("/");
        cs.close();
    }

    if (true) {
        // read the new config file
        cs.open(CONFIG_PATH);
        Serial.print("Config file after update: ");
        cs.printConfig();
        cs.close();
        Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
}

void loop() {
    delay(1000);
};
