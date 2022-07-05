/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

////#include <stdexcept.h>
#include "ConfigService.h"


//// FIXME throw exception on failure
ConfigService::ConfigService() {
    if (!lfs.mountLFS()) {
        Serial.println("ERROR: failed to mount LittleFS");
        return;
    }
}

ConfigService::~ConfigService() {
    lfs.unmountLFS();
}

ConfigService &ConfigService::getInstance() {
    static ConfigService instance;
    return(instance);
}

bool ConfigService::open(const String& configPath) {
    if (!lfs.mountLFS()) {
        Serial.println("ERROR: failed to mount LittleFS");
        return(false);
    }
    configPath.toCharArray(_configPath, sizeof(_configPath));
    if (!LittleFS.exists(_configPath)) {
        _println("WARNING: No config file; initializing config file with an empty JSON object");
        if (!initializeConfig()) {
            return(false);
        }
    }
    if (!_readConfig()) {
        Serial.println("ERROR: failed to read file: ");
        Serial.println(_configPath);
        return(false);
    }
    return(true);
}

void ConfigService::close() {
    lfs.unmountLFS();
}

bool ConfigService::initializeConfig() {
    deserializeJson(configJsonDoc, "{}");
    return(saveConfig());
}

bool ConfigService::saveConfig() {
    File f = LittleFS.open(_configPath, "w");
    if (!f) {
        Serial.printf("ERROR: failed to write config file: %s\n", _configPath);
        return(false);
    }
    serializeJson(configJsonDoc, f);
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
    configJsonDoc.clear();
    _println(String("Removed config file: ") + _configPath);
    return(true);
}

bool ConfigService::_readConfig() {
    File f = LittleFS.open(_configPath, "r");
    if (!f) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    deserializeJson(configJsonDoc, f);
    _println("Read config file and deserialized JSON");
    f.close();
    return(true);
}

void ConfigService::_displayConfigDoc() {
    serializeJsonPretty(configJsonDoc, Serial);
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

ConfigService &cs {ConfigService::getInstance()};
