/***************************************************************************
 *
 * Configuration Service Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"


#define APP_NAME        "ConfigServiceTest"
#define APP_VERSION     "2.0.0"

#define CONFIG_FILE     CONFIG_FILE0
#define CONFIG_FILE0    "/config.json"   // none
#define CONFIG_FILE1    "/config1.json"  // empty
#define CONFIG_FILE2    "/config2.json"  // partial
#define CONFIG_FILE3    "/config3.json"  // full

#define CS_DOC_SIZE     256


typedef struct {
    int     i;
    float   c;
} ConfigSubState;

ConfigSubState configSubState = {
    -5,
    5555.5
};

//// TODO include examples of using arrays and nested arrays
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

ConfigService *csPtr;
//ConfigService cs(CS_DOC_SIZE, CONFIG_FILE);


void halt() {
    while (true) {};
}


// Use values from local struct if corresponding value not found in config doc
void initConfig(ConfigService *csObj, ConfigState *structPtr) {
    DynamicJsonDocument d = *(csObj->doc);
    INIT_CONFIG(csObj, "str", structPtr->str);
    INIT_CONFIG(csObj, "i", structPtr->i);
    INIT_CONFIG(csObj, "a", structPtr->a);
    INIT_CONFIG(csObj, "b", structPtr->b);
    INIT_CONFIG(csObj, "c", structPtr->c);
    INIT_CONFIG(csObj, "d", structPtr->d);
}

// Load config doc with values from local struct
// N.B. This uses implicit casting
void setConfig(ConfigService *csObj, ConfigState *structPtr) {
    SET_CONFIG(csObj, "str", structPtr->str);
    SET_CONFIG(csObj, "i", structPtr->i);
    SET_CONFIG(csObj, "a", structPtr->a);
    SET_CONFIG(csObj, "b", structPtr->b);
    SET_CONFIG(csObj, "c", structPtr->c);
    SET_CONFIG(csObj, "d", structPtr->d);
}

// Load local struct with values from config doc
// N.B. This uses explicit casting
void getConfig(ConfigService *csObj, ConfigState *structPtr) {
    GET_CONFIG(structPtr->str, csObj, "str", String);
    GET_CONFIG(structPtr->i, csObj, "i", int);
    GET_CONFIG(structPtr->a, csObj, "a", char);
    GET_CONFIG(structPtr->b, csObj, "b", bool);
    GET_CONFIG(structPtr->c, csObj, "c", float);
    //// FIXME
//    GET_CONFIG(structPtr->d, csObj, "d", String);
}

void setup() {
    delay(500);
    Serial.begin(115200);
    delay(500);
    Serial.print("\nBEGIN: ");
    Serial.println(APP_NAME);
    Serial.println("==========================");

    if (true) {
        // should have created an empty config file as none exists after formatting
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        Serial.println("Initial contents");
        csPtr->listFiles("/");
        csPtr->printConfig();
        Serial.println("Formatting LittleFS");
        csPtr->format();
        // should be gone after formatting the fs
        csPtr->listFiles("/");
        csPtr-> printConfig();
        Serial.println("==> Expect: \"ERROR: Failed to open file for reading\"");
    }
    Serial.println("==========================");

    if (true) {
        // read and print contents of file
        Serial.println("1vvvvvvvvvvvvvvvvvvvvvvvvvv1");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        Serial.println("Expect: \"WARNING: No config file...\"");
        csPtr->listFiles("/");
        Serial.println("Contents of config file: " + String(CONFIG_FILE0));
        csPtr->printConfig();
        Serial.println("==> Expect: \"{}\"");
    }

    if (true) {
        // initialize the config file (from compiled defaults) with an empty config file
        Serial.println("2vvvvvvvvvvvvvvvvvvvvvvvvvv2");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        Serial.println("Initialize missing parts of the config");
        initConfig(csPtr, &configState);
        csPtr->saveConfig();
        csPtr->listFiles("/");
        Serial.println("Contents of config file after update: " + String(CONFIG_FILE0));
        csPtr->printConfig();
        Serial.println("==> Expect: \"{str: Test String, i: 1234, a: 97, b: true, c: 3.14, d: char array}\"");
    }

    if (true) {
        // initialize the config file with a subset of the fields
        Serial.println("3vvvvvvvvvvvvvvvvvvvvvvvvvv3");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        Serial.println("Modify a subset of the config");
        (*(csPtr->doc))["i"] = configSubState.i;
        (*(csPtr->doc))["c"] = configSubState.c;
        csPtr->saveConfig();
        csPtr->listFiles("/");
        Serial.println("Contents of config file after modification: " + String(CONFIG_FILE0));
        csPtr->printConfig();
        Serial.println("==> Expect: \"{\"str\":\"Test String\",\"i\":-5,\"a\":97,\"b\":true,\"c\":5555.5,\"d\":\"char array\"}\"");
    }

    if (true) {
        Serial.println("3vvvvvvvvvvvvvvvvvvvvvvvvvv3");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        csPtr->listFiles("/");
        Serial.print("Contents of config file: ");
        csPtr->printConfig();
        Serial.println("\nWrite empty json object to config file: " + String(CONFIG_FILE0));
        deserializeJson(*(csPtr->doc), "{}");
        csPtr->saveConfig();
        Serial.print("Contents of empty config file: ");
        csPtr->printConfig();
        Serial.println("\nInitialize with the full config");
        initConfig(csPtr, &configState);
        csPtr->saveConfig();
        csPtr->listFiles("/");
        Serial.print("Contents of config file: ");
        csPtr->printConfig();
        Serial.println("==> Expect: \"{\"str\":\"Test String\",\"i\":1234,\"a\":97,\"b\":true,\"c\":3.140000105,\"d\":\"char array\"}\"");
    }

    if (true) {
        Serial.println("5vvvvvvvvvvvvvvvvvvvvvvvvvv5");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE0);
        csPtr->listFiles("/");
        Serial.print("Contents of complete config file: ");
        csPtr->printConfig();
        Serial.println("==> Expect: \"{\"str\":\"Test String\",\"i\":1234,\"a\":97,\"b\":true,\"c\":3.140000105,\"d\":\"char array\"}\"");
        Serial.println("\nUpdate the config");
        getConfig(csPtr, &configState);
        Serial.println("Before: i=" + String(configState.i) + ", b=" + configState.b);
        configState.i++;
        configState.b = !configState.b;
        Serial.println("After: i=" + String(configState.i) + ", b=" + configState.b);
        setConfig(csPtr, &configState);
        csPtr->saveConfig();
        csPtr->listFiles("/");
        Serial.print("\nContents of config file: ");
        csPtr->printConfig();
        Serial.println("=====> Expect: {\"str\":\"Test String\",\"i\":1235,\"a\":97,\"b\":false,\"c\":3.140000105,\"d\":\"char array\"}");
    }

    /*
    if (true) {
        // read the new config file
        cs.open(CONFIG_PATH);
        Serial.print("Config file after update: ");
        cs.printConfig();
        cs.close();
        Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
    */
}

void loop() {
    delay(1000);
};
