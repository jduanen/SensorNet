/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

#include "ConfigService.h"


ConfigService::ConfigService(const String& configPath) {
    _cs = ConfigStorage(configPath);
    _configDoc = _cs.get();
    _configJson = _configDoc.as<JsonObject>();
    if (_configJson.isNull()) {
        if (VERBOSE) {
            Serial.println("No config file, initializing an empty file");
        }
        initializeConfig(configPath);
    }
    if (VERBOSE) {
        Serial.println("Read config file:");
        serializeJsonPretty(_configJson, Serial);
        Serial.println("");
    }
}

void ConfigService::initializeConfig(const String& configPath) {
    _cs.initialize();
    deserializeJson(_configJson, "{}");
    _saveConfig();
}

void ConfigService::updateConfig(const StaticJsonDocument<JSON_OBJ_SIZE>& confDoc) {
    for (JsonPair kv : confDoc) {
        Serial.println(kv.key().c_str() + ": " + kv.value());
    }
    _saveConfig();
}

void ConfigService::_saveConfig() {
    if (VERBOSE) {
        Serial.println("Save config file:");
        serializeJsonPretty(_configJson, Serial);
        Serial.println("");
    }
    _cs.set(_configJson);
    _cs.save();
}
