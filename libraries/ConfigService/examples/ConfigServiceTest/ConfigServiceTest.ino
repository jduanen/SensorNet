/***************************************************************************
 *
 * Configuration Service Library Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"


#define APP_NAME        "ConfigServiceTest"
#define APP_VERSION     "2.1.0"

#define CONFIG_FILE     "/config.json"   // none
#define CONFIG_FILE1    "/config1.json"  // empty
#define CONFIG_FILE2    "/config2.json"  // partial {"i": 9999, "b": false, "c": 1.234}
#define CONFIG_FILE3    "/config3.json"  // partial {"str": "TEST STR", "i": 987654321, "b": false, "c": 1.0}
#define CONFIG_FILE4    "/config4.json"  // partial {"str": "TEST STR", "i": 987654321, "a": "z", "b": false, "c": 1.0, "tuples": [...]}
#define CONFIG_FILE5    "/config5.json"  // full
#define CONFIG_FILE6    "/config6.json"  // excess tuples

#define EXPECT1         "{\"str\":\"Test String\",\"i\":1234,\"b\":1,\"c\":3.14,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"
#define EXPECT2         "{\"str\":\"Test String\",\"i\":9999,\"b\":0,\"c\":1.11,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"
#define EXPECT3         "{\"str\":\"TEST STR\",\"i\":987654321,\"b\":0,\"c\":2.22,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"
#define EXPECT4         "{\"str\":\"TEST STR\",\"i\":987654321,\"b\":0,\"c\":3.33,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"
#define EXPECT5         "{\"str\":\"TEST STR\",\"i\":987654321,\"b\":0,\"c\":4.44,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"
#define EXPECT6         "{\"str\":\"TEST STR\",\"i\":987654321,\"b\":0,\"c\":5.55,\"tuples\":[[0,0],[1,11],[2,22],[3,33],[4,44],[5,55],[6,66],[7,77],[8,88],[9,99],[10,100],[11,110],[12,120],[13,130],[14,140],[15,150]]}"

#define CS_DOC_SIZE     1536

#define NUM_TUPLES      16


typedef struct {
    int     i;
    float   c;
} ConfigSubState;

ConfigSubState configSubState = {
    -5,
    5555.5
};

//// TODO include examples of using arrays and nested arrays
//// FIXME make char and char[] work
typedef struct {
    String      str;
    int         i;
//    char        a;
    bool        b;
    float       c;
//    char        d[];
    uint32_t    tuples[NUM_TUPLES][2];
} ConfigState;

ConfigState configState = {
    String("Test String"),
    1234,
//    'a',
    true,
    3.14,
//    "char array",
    {
        {0, 0},
        {1, 11},
        {2, 22},
        {3, 33},
        {4, 44},
        {5, 55},
        {6, 66},
        {7, 77},
        {8, 88},
        {9, 99},
        {10, 100},
        {11, 110},
        {12, 120},
        {13, 130},
        {14, 140},
        {15, 150}
    }
};

ConfigService *csPtr;


void halt() {
    Serial.println("HALT");
    while (true) {
        wdt_reset();
        delay(100);
    }
};

void printTuples(String hdr, uint32_t tuples[][2]) {
    Serial.print(hdr);
    //// use forall/iterator instead?
    for (int i = 0; (i < NUM_TUPLES); i++) {
        if (i > 0) {
            Serial.print(", ");
        }
        Serial.print("[0x" + String(tuples[i][0], HEX) + ", 0x" + String(tuples[i][1], HEX) + "]");
    }
    Serial.println("]");
};

void printConfigState(ConfigState *structPtr) {
    Serial.println("configState:");
    Serial.println("    str:\t " + structPtr->str);
    Serial.println("    i:\t\t " + String(structPtr->i));
//    Serial.println("    a:\t\t " + String(structPtr->a));
    Serial.println("    b:\t\t " + String(structPtr->b));
    Serial.println("    c:\t\t " + String(structPtr->c));
//    Serial.println("    d:\t\t " + String(structPtr->d));
    printTuples("    tuples:\t ", structPtr->tuples);
};

// Use values from local struct if corresponding value not found in config doc
void initConfig(ConfigService *csPtr, ConfigState *structPtr) {
    INIT_STATE(csPtr, "str", structPtr->str, String);
    INIT_STATE(csPtr, "i", structPtr->i, int);
//    INIT_STATE(csPtr, "a", structPtr->a, unsigned char);
    INIT_STATE(csPtr, "b", structPtr->b, bool);
    INIT_STATE(csPtr, "c", structPtr->c, float);
//    INIT_STATE(csPtr, "d", structPtr->d, String);
    uint16_t numTuples = (*(csPtr->docPtr))["tuples"].size();
    //Serial.println("> #Tuples: " + String(numTuples) + ", OVFL: " + String(csPtr->docPtr->overflowed()) + ", VALID: " + csPtr->validEntry("tuples"));
    if (!csPtr->validEntry("tuples")) {
        csPtr->docPtr->createNestedArray("tuples");
        numTuples = 0;
    }
    //Serial.println(">> #Tuples: " + String(numTuples) + ", OVFL: " + String(csPtr->docPtr->overflowed()) + ", VALID: " + csPtr->validEntry("tuples"));
    if (numTuples != NUM_TUPLES) {
        Serial.println("ERROR: wrong number of tuples");
        //// FIXME
    }
    if (csPtr->validEntry("tuples") && (numTuples == NUM_TUPLES)) {
        copyArray((*(csPtr->docPtr))["tuples"], structPtr->tuples);
        if (csPtr->docPtr->overflowed()) {
            Serial.println("ERROR: overflowed config JsonDocument");
        }
        numTuples = (*(csPtr->docPtr))["tuples"].size();
        if (numTuples != NUM_TUPLES) {
            Serial.println("ERROR: incorrect number of tuples: " + String(numTuples));
        }
    }
    //Serial.println(">>> #Tuples: " + String(numTuples) + ", OVFL: " + String(csPtr->docPtr->overflowed()) + ", VALID: " + csPtr->validEntry("tuples"));
};

// Write state values from local struct to config file
void saveConfig(ConfigService *csPtr, ConfigState *structPtr) {
    String jsonStr = "{";
    jsonStr.concat("\"str\": \"" + structPtr->str + "\", ");
    jsonStr.concat("\"i\": " + String(structPtr->i) + ", ");
//    jsonStr.concat("\"a\": \'" + String(structPtr->a) + "\', ");
    jsonStr.concat("\"b\": " + String(structPtr->b ? true : false) + ", ");
    jsonStr.concat("\"c\": " + String(structPtr->c) + ", ");
    //// FIXME deal with char arrays
    jsonStr.concat("\"tuples\": [");
    for (int i = 0; (i < NUM_TUPLES); i++) {
        if (i > 0) {
            jsonStr.concat(", ");
        }
        jsonStr.concat("[" + String(structPtr->tuples[i][0]) + ", " + String(structPtr->tuples[i][1]) + "]");
    }
    jsonStr.concat("]");
    jsonStr.concat("}");
    if (!csPtr->setConfig(jsonStr)) {
        Serial.println("ERROR: failed to save config");
    }
};

void printConfFiles() {
    String fileNames[] = {CONFIG_FILE1, CONFIG_FILE2, CONFIG_FILE3, CONFIG_FILE4, CONFIG_FILE5, CONFIG_FILE6};
    ConfigService *csp;

    csp = new ConfigService(CS_DOC_SIZE, CONFIG_FILE);
    csp->listFiles("/");
    Serial.println("Contents of config file: " + String(CONFIG_FILE));
    csp->printConfigFile();

    for (int i = 0; (i < (sizeof(fileNames) / sizeof(fileNames[0]))); i++) {
        csp = new ConfigService(CS_DOC_SIZE, fileNames[i]);
        Serial.println("Contents of config file #" + String(i + 1) + ": " + fileNames[i]);
        csp->printConfigFile();
    }
};

void setup() {
    delay(500);
    Serial.begin(115200);
    delay(500);
    Serial.print("\nBEGIN: "); Serial.println(APP_NAME);
    Serial.println("VVVVVVVVVVVVVVVVVVVVVVVVVVV");
    printConfFiles();

    if (false) {
        // should have created an empty config file as none exists after formatting
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE);
        Serial.println("Initial contents");
        csPtr->listFiles("/");
        csPtr->printConfigFile();
        Serial.println("Formatting LittleFS");
        csPtr->formatFS();
        // should be gone after formatting the fs
        csPtr->listFiles("/");
        csPtr->printConfigFile();
        Serial.println("======> Expect: \"ERROR: Failed to open file for reading\"");
    }
    Serial.println("==========================");

    if (false) {
        // read and print contents of file
        Serial.println("0vvvvvvvvvvvvvvvvvvvvvvvvvv0");
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE1);
        Serial.println("Expect: \"WARNING: No config file...\"");
        csPtr->listFiles("/");
        Serial.println("Contents of config file: " + String(CONFIG_FILE1));
        csPtr->printConfigFile();
        Serial.println("======> Expect: \"{}\"");
    }

    String fileNames[] = {CONFIG_FILE1, CONFIG_FILE2, CONFIG_FILE3, CONFIG_FILE4, CONFIG_FILE5, CONFIG_FILE6};
    String expects[] = {EXPECT1, EXPECT2, EXPECT3, EXPECT4, EXPECT5, EXPECT6};
    for (int i = 0; (i < (sizeof(fileNames) / sizeof(fileNames[0]))); i++) {
        Serial.println(String(i + 1) + "vvvvvvvvvvvvvvvvvvvvvvvvvv" + String(i + 1));
        csPtr = new ConfigService(CS_DOC_SIZE, fileNames[i]);
        Serial.println("Initialize missing parts of the config");
        initConfig(csPtr, &configState);
//        printConfigState(&configState);

        // save config state to config file
        csPtr = new ConfigService(CS_DOC_SIZE, CONFIG_FILE);
        saveConfig(csPtr, &configState);
        csPtr->saveConfig();
        csPtr->listFiles("/");
        Serial.println("Contents of config file after partial initialization: " + fileNames[i]);
        csPtr->printConfigFile();
        Serial.println("======> Expect: " + expects[i]);
    }
    Serial.println("==========================");

    printConfFiles();
    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

void loop() {
    delay(1000);
};
