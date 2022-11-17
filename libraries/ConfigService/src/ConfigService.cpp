/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

////#include <stdexcept.h>
#include "ConfigService.h"


ConfigService::ConfigService(uint32_t docSize, const String& configFilePath) {
    doc = new DynamicJsonDocument(docSize);
    if (!lfs.mountLFS()) {
        Serial.println("ERROR: failed to mount LittleFS");
        //// FIXME throw exception on failure
        return;
    }
    configFilePath.toCharArray(_configPath, sizeof(_configPath));
    if (!LittleFS.exists(_configPath)) {
        _println("WARNING: No config file; initializing config file with an empty JSON object");
        if (!initializeConfig()) {
            return;
        }
    }
    if (!_readConfig()) {
        Serial.println("ERROR: failed to read file: ");
        Serial.println(_configPath);
        return;
    }
}

ConfigService::~ConfigService() {
    lfs.unmountLFS();
}

void ConfigService::format() {
    lfs.formatLFS();
}

void ConfigService::listFiles(const String& path) {
    lfs.listFilesLong(path);
}

bool ConfigService::initializeConfig() {
    DeserializationError error = deserializeJson(*doc, "{}");
    if (error) {
        Serial.print("ERROR: initializeConfig() deserializeJson() failed: ");
        Serial.println(error.f_str());
    };
    return(saveConfig());
}

bool ConfigService::saveConfig() {
    File f = LittleFS.open(_configPath, "w");
    if (!f) {
        Serial.printf("ERROR: failed to write config file: %s\n", _configPath);
        return(false);
    }
    serializeJson(*doc, f);
    if (_verbose) {
        Serial.printf("Wrote serialized JSON to config file: %s\n", _configPath);
        printConfig();
        Serial.println("");
    }
    f.close();
    return(true);
}

void ConfigService::printConfig() {
    _println(String("Read and print config file: ") + _configPath);
    lfs.printFile(_configPath);
}

bool ConfigService::deleteConfig() {
    if (!LittleFS.remove(_configPath)) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    doc->clear();
    _println(String("Removed config file: ") + _configPath);
    return(true);
}

bool ConfigService::validEntry(const String& key) {
    DynamicJsonDocument d = *doc;
    return ((*doc).containsKey(key) && !(*doc)[key].isNull());
}

bool ConfigService::_readConfig() {
    File f = LittleFS.open(_configPath, "r");
    if (!f) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    DeserializationError error = deserializeJson(*doc, f);
    if (error) {
        Serial.print("ERROR: _readConfig() deserializeJson() failed: ");
        Serial.println(error.f_str());
        return(false);
    };
    _println("Read config file and deserialized JSON");
    f.close();
    return(true);
}

void ConfigService::_printConfigDoc() {
    serializeJsonPretty(*doc, Serial);
}

void ConfigService::_print(String str) {
  if (_verbose) {
    Serial.print(str);
  }
}

void ConfigService::_println(String str) {
  if (_verbose) {
    Serial.println(str);
  }
}
