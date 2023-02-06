/***************************************************************************
 *
 * Library that handles application configuration state
 * 
 * N.B. config files are assumed to be <1KB, so use StaticJsonDocument
 *       which stores things on the stack.
 *
 ***************************************************************************/

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "LfsUtilities.h"


#define CONFIG_SERVICE_VERSION  "2.1"

#define MAX_PATH_LENGTH         32

#define CS_DOC(cs)                              (*(cs.docPtr))
#define CSPTR_DOC(csPtr)                        (*(csPtr->docPtr))
#define GET_CONFIG(csPtr, field, value, typ)    value = (*(csPtr->docPtr))[field].as<typ>()
#define INIT_STATE(csPtr, field, value, typ)    if (csPtr->validEntry(field)) {GET_CONFIG(csPtr, field, value, typ);}
//#define INIT_STATE(csPtr, field, value, typ)    Serial.print(field); if (csPtr->validEntry(field)) {Serial.println(" from file"); GET_CONFIG(csPtr, field, value, typ);} else {Serial.println(" from defaults");}

#define JSON_START(str)         str = "{"
/*
#define JSON_ADD_STRING(str)    ?
#define JSON_ADD_INT(str)       ?
#define JSON_ADD_BOOL(str)      ?
#define JSON_ADD_FLOAT(str)     ?
*/
#define JSON_END(str)           str.concat("}")


class ConfigService {
public:
    String libVersion = CONFIG_SERVICE_VERSION;
    DynamicJsonDocument *docPtr;

    ConfigService(uint32_t docSize, const String& configFilePath);
    ~ConfigService();
    static ConfigService &getInstance();

    void formatFS();
    void listFiles(const String& path);

    bool validEntry(const String& key);
    uint32_t configSize();

    template <class T>
    T getConfigValue(String field, T value);

    bool setConfig(const String& json);
    bool initializeConfig();
    bool saveConfig();
    void printConfigFile();
    bool deleteConfigFile();

private:
    bool _verbose = false;
    char _configPath[MAX_PATH_LENGTH];

    bool _readConfigFile();
    void _printConfigDoc();

    void _print(String str);
    void _println(String str);
};

#include "ConfigService.hpp"
