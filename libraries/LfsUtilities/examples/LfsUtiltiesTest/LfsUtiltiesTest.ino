/***************************************************************************
 *
 * LittleFS Utilities Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "LfsUtilities.h"


#define APP_NAME        "lfsTest"
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
        lfs.formatLFS();
    }
    Serial.println("vvvvvvvvvvvvvvvvvvvvvvvv");

    if (!lfs.mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    Serial.println("List Directory:");
    lfs.listFilesLong("/");
    lfs.unmountLFS();
    Serial.println("------------------------");

    if (!lfs.mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    lfs.listFilesLong("/");
    lfs.printFile(f1);
    lfs.writeFile(f1, "Hello ");
    lfs.listFilesLong("/");
    lfs.printFile(f1);
    lfs.appendFile(f1, "World!\n");
    lfs.listFilesLong("/");
    lfs.printFile(f1);
    lfs.unmountLFS();
    Serial.println("------------------------");

    if (!lfs.mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    lfs.listFilesLong("/");
    lfs.printFile(f1);
    Serial.println("Adding new file: " + f3);
    lfs.writeFile(f3, "New File");
    lfs.unmountLFS();
    Serial.println("------------------------");

    if (!lfs.mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    lfs.listFilesLong("/");
    Serial.println("New file contents:");
    lfs.printFile(f3);
    Serial.println("Rename file: " + f1 + " to " + f2);
    lfs.renameFile(f1, f2);
    lfs.listFilesLong("/");
    lfs.printFile(f2);
    Serial.println("Delete file: " + f2);
    lfs.deleteFile(f2);
    lfs.listFilesLong("/");
    lfs.unmountLFS();
    Serial.println("------------------------");

    if (!lfs.mountLFS()) {
        Serial.println("LittleFS mount failed");
        halt();
    }
    Serial.println("listFilesLong:");
    lfs.listFilesLong("/");
    lfs.unmountLFS();
    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

void loop() {
};
