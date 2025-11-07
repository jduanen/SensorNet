/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

ConfigService::ConfigService(uint32_t docSize, const String& configFilePath) {
    int maxFreeBlkSize = ESP.getMaxFreeBlockSize();
    if (maxFreeBlkSize < docSize) {
        Serial.println("ERROR: insufficiently large free block -- (" + String(maxFreeBlkSize) + ", " + String(docSize) + ")");
        //// FIXME throw exception on failure
        return;
    }
    docPtr = new DynamicJsonDocument(docSize);
    if (!lfs.mountLFS()) {
        Serial.println("ERROR: failed to mount LittleFS");
        //// FIXME throw exception on failure
        return;
    }
    configFilePath.toCharArray(_configPath, sizeof(_configPath));
    if (!LittleFS.exists(_configPath)) {
        _println("WARNING: No config file; initializing config file with an empty JSON object");
        if (!initializeConfig()) {
            Serial.println("ERROR: unable to initialize empty config file");
            //// FIXME throw an exception
            return;
        }
    }
    if (!_readConfigFile()) {
        Serial.print("ERROR: failed to read file: "); Serial.println(_configPath);
        //// FIXME throw an exception
        return;
    }
};

ConfigService::~ConfigService() {
    lfs.unmountLFS();
};

void ConfigService::formatFS() {
    lfs.formatLFS();
};

void ConfigService::listFiles(const String& path) {
    lfs.listFilesLong(path);
};

bool ConfigService::validEntry(const String& key) {
    return (docPtr->containsKey(key) && !(*docPtr)[key].isNull());
};

uint32_t ConfigService::configSize() {
    return(docPtr->memoryUsage());
};

template <class T>
T ConfigService::getConfigValue(String field, T defaultValue) {
    T v;
    if ((*(docPtr))[field]) {
        v = (*(docPtr))[field].as<T>();
    } else {
        v = defaultValue;
    }
    return(v);
};

bool ConfigService::setConfig(const String& json) {
    DeserializationError error = deserializeJson(*docPtr, json);
    if (error) {
        Serial.print("ERROR: setConfig() deserializeJson() failed: ");
        Serial.println(error.f_str());
        return(false);
    };
    return(saveConfig());
};

bool ConfigService::initializeConfig() {
    return(setConfig("{}"));
};

bool ConfigService::saveConfig() {
    File f = LittleFS.open(_configPath, "w");
    if (!f) {
        Serial.printf("ERROR: failed to write config file: %s\n", _configPath);
        return(false);
    }
    size_t bytesWritten = serializeJson(*docPtr, f);
    if (_verbose) {
        Serial.printf("Wrote serialized JSON to config file: %s (%d Bytes)\n", _configPath, bytesWritten);
        printConfigFile();
        Serial.println("");
    }
    f.close();
    return(true);
};

void ConfigService::printConfigFile() {
    _println(String("Read and print config file: ") + _configPath);
    lfs.printFile(_configPath);
};

bool ConfigService::deleteConfigFile() {
    if (!LittleFS.remove(_configPath)) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    docPtr->clear();
    _println(String("Removed config file: ") + _configPath);
    return(true);
};

bool ConfigService::_readConfigFile() {
    File f = LittleFS.open(_configPath, "r");
    if (!f) {
        Serial.printf("ERROR: failed to read config file: %s\n", _configPath);
        return(false);
    }
    DeserializationError error = deserializeJson(*docPtr, f);
    if (error) {
        Serial.print("ERROR: _readConfigFile() deserializeJson() failed: ");
        Serial.println(error.f_str());
        return(false);
    };
    if (docPtr->overflowed()) {
        Serial.println("ERROR: doc overflowed on read from file");
        return(false);
    }
    _println("Read config file and deserialized JSON");
    f.close();
    return(true);
};

void ConfigService::_printConfigDoc() {
    serializeJsonPretty(*docPtr, Serial);
};

void ConfigService::_print(String str) {
  if (_verbose) {
    Serial.print(str);
  }
};

void ConfigService::_println(String str) {
  if (_verbose) {
    Serial.println(str);
  }
};
