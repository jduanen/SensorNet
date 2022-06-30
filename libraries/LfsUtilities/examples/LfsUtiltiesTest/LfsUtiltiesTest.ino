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

    if (true) {
        Serial.println("Formatting LittleFS filesystem");
        formatLFS();
    }

    Serial.println("vvvvvvvvvvvvvvvvvvvvvvvv");

    Serial.println("Mount LittleFS");
    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    Serial.println("List Directory:");
    listDir("/");
    Serial.println("Unmount LittleFS");
    unmountLFS();

    Serial.println("------------------------");

    Serial.println("Mount LittleFS");
    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    listDir("/");
    deleteFile("/hello.txt");
    listDir("/");
    writeFile("/hello.txt", "Hello ");
    listDir("/");
    appendFile("/hello.txt", "World!\n");
    listDir("/");
    Serial.println("Unmount LittleFS");
    unmountLFS();

    Serial.println("------------------------");

    Serial.println("Mount LittleFS");
    if (!mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    listDir("/");
    printFile("/hello.txt");
    Serial.println("Unmount LittleFS");
    unmountLFS();

    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

void loop() {
};
