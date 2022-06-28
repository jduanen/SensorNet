/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#include "LfsUtilities.h"


/*
//// TODO make this take indent level arg
        String paths[] = {COMMON_PAGE_PATH, COMMON_STYLE_PATH, COMMON_SCRIPT_PATH};
        for (int i = 0; (i < NUM_ITEMS(paths)); i++) {
            Serial.print(paths[i]);
            if (LittleFS.exists(paths[i]) == false) {
                Serial.println(": file not found");
            } else {
                Serial.println(": file exists");
            }
        }
*/

void listFiles(String dirPath) {
    //// TODO add '/' if dirPath doesn't end with one
    Dir d = LittleFS.openDir(dirPath);
    while (d.next()) {
        if (d.isFile()) {
            Serial.println("File: " + dirPath + d.fileName());
        } else if (d.isDirectory()) {
            String p = dirPath + d.fileName() + "/";
            Serial.println("Dir: " + p);
            listFiles(p);
        } else {
            Serial.println("Unknown: " + d.fileName());
        }
    }
}

void formatLFS() {
    Serial.println("Formatting -- erasing all local files");
    LittleFS.format();
}
