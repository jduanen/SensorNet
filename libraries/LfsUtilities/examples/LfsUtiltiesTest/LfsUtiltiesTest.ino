/***************************************************************************
 *
 * LittleFS Utilities Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "LfsUtilities.h"


#define VERBOSE         1
#define APP_NAME        "LfsUtilitiesTest"
#define APP_VERSION     "1.0.0"


void halt() {
    while (true) {};
}

void setup() {
    delay(500);
    Serial.begin(19200);
    delay(500);
    Serial.print("\nBEGIN: ");
    Serial.println(APP_NAME);

    const String f1 = "/hello.txt";
    const String f2 = "/goodbye.txt";
    const String f3 = "/newfile.txt";

    if (true) {
        formatLFS();
    }
    Serial.println("vvvvvvvvvvvvvvvvvvvvvvvv");

    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    Serial.println("List Directory:");
    listDir("/");
    unmountLFS();
    Serial.println("------------------------");

    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    listDir("/");
    printFile(f1);
    writeFile(f1, "Hello ");
    listDir("/");
    printFile(f1);
    appendFile(f1, "World!\n");
    listDir("/");
    printFile(f1);
    unmountLFS();
    Serial.println("------------------------");

    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    listDir("/");
    printFile(f1);
    Serial.println("Adding new file: " + f3);
    writeFile(f3, "New File");
    unmountLFS();
    Serial.println("------------------------");

    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    listDir("/");
    Serial.println("New file contents:");
    printFile(f3);
    Serial.println("Rename file: " + f1 + " to " + f2);
    renameFile(f1, f2);
    listDir("/");
    printFile(f2);
    Serial.println("Delete file: " + f2);
    deleteFile(f2);
    listDir("/");
    unmountLFS();
    Serial.println("------------------------");

    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    Serial.println("ListFiles:");
    listFiles("/");
    unmountLFS();
    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

void loop() {
};
