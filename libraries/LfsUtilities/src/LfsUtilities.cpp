/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#include <time.h>
#include "LfsUtilities.h"


LfsUtilities::LfsUtilities() {
    mountLFS();
}

LfsUtilities::~LfsUtilities() {
    unmountLFS();
}

LfsUtilities &LfsUtilities::getInstance() {
    static LfsUtilities instance;
    return(instance);
}

void LfsUtilities::formatLFS() {
    _println("Formatting -- erasing all local files");
    LittleFS.format();
}

bool LfsUtilities::mountLFS() {
    bool r = LittleFS.begin();
    if (!r) {
        Serial.println("ERROR: failed to mount LittleFS");
    }
    _println("Mounted LittleFS");
    return(r);
}

void LfsUtilities::unmountLFS() {
    LittleFS.end();
    _println("Unmounted LittleFS");
}

bool LfsUtilities::printFile(const String& path) {
    _println("Reading file: " + path);
    File f = LittleFS.open(path, "r");
    if (!f) {
        Serial.println("ERROR: Failed to open file for reading");
        return(false);
    }
    _print("Read from file: ");
    while (f.available()) {
        Serial.write(f.read());
    }
    Serial.print("\n");
    f.close();
    return(true);
}

bool LfsUtilities::writeFile(const String& path, const char *contents) {
    _println("Writing file: " + path);
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
    _println("File written");
    delay(2000); // Make sure the CREATE and LASTWRITE times are different
    f.close();
    return(true);
}

bool LfsUtilities::appendFile(const String& path, const char *contents) {
    _println("Appending to file: " + path);
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
    _println("Message appended");
    f.close();
    return(true);
}

bool LfsUtilities::renameFile(const String& srcPath, const String& dstPath) {
    _println("Renaming file " + srcPath + " to " + dstPath);
    if (!LittleFS.rename(srcPath, dstPath)) {
        Serial.println("ERROR: Rename failed");
        return(false);
    }
    _println("File renamed");
    return(true);
}

bool LfsUtilities::deleteFile(const String& path) {
    _println("Deleting file: " + path);
    if (!LittleFS.remove(path)) {
        Serial.println("ERROR: Delete failed");
        return(false);
    }
    _println("File deleted");
    return(true);
}

/*
int LfsUtilities::getFiles(const String& dirName, char *paths) {
    //// TODO return number of files and list of files in paths
    Serial.println("TBD");
    return(0);
}
*/

void LfsUtilities::listFiles(const String& dirPath) {
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

//// FIXME must initialize localtime
void LfsUtilities::listFilesLong(const String& dirPath, uint8_t indent) {
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

String LfsUtilities::_indent(uint8_t num) {
    //// TODO assert num >= 0
    char s[num];
    for (int i = 0; (i < num); i++) {
        s[i] = ' ';
    }
    s[num] = 0;
    return(String(s));
}

void LfsUtilities::_print(String str) {
  if (_verbose) {
    Serial.print(str);
  }
}

void LfsUtilities::_println(String str) {
  if (_verbose) {
    Serial.println(str);
  }
}


LfsUtilities &lfs {LfsUtilities::getInstance()};
