/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#include <time.h>
#include "LfsUtilities.h"


void formatLFS() {
    if (VERBOSE) {
        Serial.println("Formatting -- erasing all local files");
    }
    LittleFS.format();
}

bool mountLFS() {
    bool r = LittleFS.begin();
    if (!r) {
        Serial.println("ERROR: failed to mount LittleFS");
    }
    if (VERBOSE) {
        Serial.println("Mounted LittleFS");
    }
    return(r);
}

void unmountLFS() {
    LittleFS.end();
    if (VERBOSE) {
        Serial.println("Unmounted LittleFS");
    }
}

bool printFile(const String& path) {
    Serial.println("Reading file: " + path);
    File f = LittleFS.open(path, "r");
    if (!f) {
        Serial.println("ERROR: Failed to open file for reading");
        return(false);
    }
    Serial.print("Read from file: ");
    while (f.available()) {
        Serial.write(f.read());
    }
    Serial.print("\n");
    f.close();
    return(true);
}

bool writeFile(const String& path, const char *contents) {
    if (VERBOSE) {
        Serial.println("Writing file: " + path);
    }
    File f = LittleFS.open(path, "w");
    if (!f) {
        Serial.print("ERROR: Failed to open file for writing: ");
        Serial.println(path);
        return(false);
    }
    if (!f.print(contents)) {
        Serial.print("ERROR: Write failed: ");
        Serial.println(path);
        f.close();
        return(false);
    }
    if (VERBOSE) {
        Serial.println("File written");
    }
    delay(2000); // Make sure the CREATE and LASTWRITE times are different
    f.close();
    return(true);
}

bool appendFile(const String& path, const char *contents) {
    if (VERBOSE) {
        Serial.println("Appending to file: " + path);
    }
    File f = LittleFS.open(path, "a");
    if (!f) {
        Serial.print("ERROR: Failed to open file for appending: ");
        Serial.println(path);
        return(false);
    }
    if (!f.print(contents)) {
        Serial.println("ERROR: Append failed: " + path);
        f.close();
        return(false);
    }
    if (VERBOSE) {
        Serial.println("Message appended");
    }
    f.close();
    return(true);
}

bool renameFile(const String& srcPath, const String& dstPath) {
    if (VERBOSE) {
        Serial.println("Renaming file " + srcPath + " to " + dstPath);
    }
    if (!LittleFS.rename(srcPath, dstPath)) {
        Serial.println("ERROR: Rename failed");
        return(false);
    }
    if (VERBOSE) {
        Serial.println("File renamed");
    }
    return(true);
}

bool deleteFile(const String& path) {
    if (VERBOSE) {
        Serial.println("Deleting file: " + path);
    }
    if (!LittleFS.remove(path)) {
        Serial.println("ERROR: Delete failed");
        return(false);
    }
    if (VERBOSE) {
        Serial.println("File deleted");
    }
    return(true);
}

int getFiles(const String& dirName, char *paths) {
    //// TODO return number of files and list of files in paths
    Serial.println("TBD");
    return(0);
}

void listFiles(const String& dirPath) {
    Dir dir = LittleFS.openDir(dirPath);
    while (dir.next()) {
        if (dir.isFile()) {
            Serial.println(dirPath + dir.fileName());
        }
        if (dir.isDirectory()) {
            Serial.println(dirPath + dir.fileName() + "/");
            listFiles(dirPath + dir.fileName() + "/");
        }
    }
}

String _indent(uint8_t num) {
    //// TODO assert num >= 0
    char s[num];
    for (int i = 0; (i < num); i++) {
        s[i] = ' ';
    }
    s[num] = 0;
    return(String(s));
}

//// FIXME must initialize localtime
//// TODO add arg for indent and indent directories
void listFilesLong(const String& dirPath, uint8_t indent) {
    Dir root = LittleFS.openDir(dirPath);
    while (root.next()) {
        File f = root.openFile("r");
        if (root.isFile()) {
            Serial.print(_indent(indent) + root.fileName() + " \t");
        } else if (root.isDirectory()) {
            Serial.println(_indent(indent) + root.fileName() + "/ \t");
            listFilesLong(dirPath + root.fileName() + "/", (indent + 2));
            continue;
        } else {
            Serial.print("ERROR: Unknown file type: " + root.fileName());
            continue;
        }
        Serial.print("Size: " + String(f.size()) + " \t");
        time_t cr = f.getCreationTime();
        time_t lw = f.getLastWrite();
        f.close();
        struct tm * tmstruct = localtime(&cr);
        Serial.printf("Creation: %d-%02d-%02d %02d:%02d:%02d \t",
                      (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                      tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min,
                      tmstruct->tm_sec);
        tmstruct = localtime(&lw);
        Serial.printf("Last Write: %d-%02d-%02d %02d:%02d:%02d\n",
                      (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                      tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min,
                      tmstruct->tm_sec);
    }
}

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
