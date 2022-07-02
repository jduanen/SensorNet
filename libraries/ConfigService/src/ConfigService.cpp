/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

////#include <stdexcept.h>
#include "ConfigService.h"


//// FIXME throw exception on failure
ConfigService::ConfigService(const String& configPath) {
    configPath.toCharArray(_configPath, sizeof(_configPath));
    if (!mountLFS()) {
        Serial.println("ERROR: failed to mount LittleFS");
        return;
    }
    if (!LittleFS.exists(_configPath)) {
        if (VERBOSE) {
            Serial.println("WARNING: No config file; initializing config file with an empty JSON object");
        }
        if (!initializeConfig()) {
            return;
        }
    }
    if (!_readConfig()) {
        return;
    }
}

ConfigService::~ConfigService() {
    unmountLFS();
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
    if (VERBOSE) {
        Serial.printf("Wrote serialized JSON to config file: %s\n", _configPath);
        printConfig();
        Serial.println("");
    }
    f.close();
    return(true);
}

void ConfigService::printConfig() {
    if (VERBOSE) {
        Serial.printf("Read and print config file: %s\n", _configPath);
    }
    printFile(_configPath);
}

bool ConfigService::deleteConfig() {
    if (!LittleFS.remove(_configPath)) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    configJsonDoc.clear();
    if (VERBOSE) {
        Serial.printf("Removed config file: %s\n", _configPath);
    }
    return(true);
}

bool ConfigService::_readConfig() {
    File f = LittleFS.open(_configPath, "r");
    if (!f) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    deserializeJson(configJsonDoc, f);
    if (VERBOSE) {
        Serial.println("Read config file and deserialized JSON");
    }
    f.close();
    return(true);
}

void ConfigService::_displayConfigDoc() {
    serializeJsonPretty(configJsonDoc, Serial);
}
