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

//// FIXME must initialize localtime
void listDir(const char *dirname) {
    Serial.printf("Listing directory: %s\n", dirname);
    Dir root = LittleFS.openDir(dirname);
    while (root.next()) {
        File f = root.openFile("r");
        Serial.printf("  FILE: %s", root.fileName());
        Serial.printf("  SIZE: %d", f.size());
        time_t cr = f.getCreationTime();
        time_t lw = f.getLastWrite();
        f.close();
        struct tm * tmstruct = localtime(&cr);
        Serial.printf("  CREATION: %d-%02d-%02d %02d:%02d:%02d ",
                      (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                      tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min,
                      tmstruct->tm_sec);
        tmstruct = localtime(&lw);
        Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                      (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                      tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min,
                      tmstruct->tm_sec);
    }
}

bool printFile(const char *path) {
    Serial.printf("Reading file: %s\n", path);
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

bool writeFile(const char *path, const char *contents) {
    if (VERBOSE) {
        Serial.printf("Writing file: %s\n", path);
    }
    File f = LittleFS.open(path, "w");
    if (!f) {
        Serial.printf("ERROR: Failed to open file for writing: %s\n", path);
        return(false);
    }
    if (!f.print(contents)) {
        Serial.printf("ERROR: Write failed: %s\n", path);
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

bool appendFile(const char *path, const char *contents) {
    if (VERBOSE) {
        Serial.printf("Appending to file: %s\n", path);
    }
    File f = LittleFS.open(path, "a");
    if (!f) {
        Serial.printf("ERROR: Failed to open file for appending: %s\n", path);
        return(false);
    }
    if (!f.print(contents)) {
        Serial.printf("ERROR: Append failed: %s\n", path);
        f.close();
        return(false);
    }
    if (VERBOSE) {
        Serial.println("Message appended");
    }
    f.close();
    return(true);
}

bool renameFile(const char *srcPath, const char *dstPath) {
    if (VERBOSE) {
        Serial.printf("Renaming file %s to %s\n", srcPath, srcPath);
    }
    if (!LittleFS.rename(srcPath, srcPath)) {
        Serial.println("ERROR: Rename failed");
        return(false);
    }
    if (VERBOSE) {
        Serial.println("File renamed");
    }
    return(true);
}

bool deleteFile(const char *path) {
    if (VERBOSE) {
        Serial.printf("Deleting file: %s\n", path);
    }
    if (LittleFS.remove(path)) {
        Serial.println("ERROR: Delete failed");
        return(false);
    }
    if (VERBOSE) {
        Serial.println("File deleted");
    }
    return(true);
}

int getFiles(const char *dirname, char *paths) {
    //// TODO return number of files and list of files in paths
    Serial.println("TBD");
    return(0);
}

/*
void listFiles(const String& dirPath) {
    //// TODO add '/' if dirPath doesn't end with one
    Dir d = LittleFS.openDir(dirPath);
    Serial.println("LF: " + dirPath);
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
    Serial.println("DONE");
}
*/

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
