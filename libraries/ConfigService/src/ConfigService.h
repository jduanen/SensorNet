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

/*
#define INIT_CONFIG_PTR(csPtr, field, value)        if (!csPtr->validEntry(field)) {(*(csPtr->docPtr))[field] = value;}
#define SET_CONFIG_PTR(csPtr, field, value)         (*(csPtr->docPtr))[field] = value
#define GET_CONFIG_PTR(value, csPtr, field, typ)    value = (*(csPtr->docPtr))[field].as<typ>()

#define INIT_CONFIG(csObj, field, value)            if (!csObj.validEntry(field)) {(*(csObj.docPtr))[field] = value;}
#define SET_CONFIG(csObj, field, value)             (*(csObj.docPtr))[field] = value
*/
#define CS_DOC(cs)                              (*(cs.docPtr))
#define CSPTR_DOC(csPtr)                        (*(csPtr->docPtr))
#define GET_CONFIG(csPtr, field, value, typ)    value = (*(csPtr->docPtr))[field].as<typ>()
//#define INIT_STATE(csPtr, field, value, typ)    if (csPtr->validEntry(field)) {GET_CONFIG(csPtr, field, value, typ);}
#define INIT_STATE(csPtr, field, value, typ)    Serial.print(field); if (csPtr->validEntry(field)) {Serial.println(" from file"); GET_CONFIG(csPtr, field, value, typ);} else {Serial.println(" from defaults");}

//// FIXME -- types: string, char, int/float, bool, tuples
/*
#define ADD_STRING_FIELD(jsonStr, field, value) jsonStr.concat(field: value)
#define ADD_INT_FIELD(jsonStr, field, value)    jsonStr.concat(field: value)
#define ADD_CHAR_FIELD(jsonStr, field, value)   jsonStr.concat(field: value)
#define ADD_BOOL_FIELD(jsonStr, field, value)   jsonStr.concat(field: value)
#define ADD_FLOAT_FIELD(jsonStr, field, value)  jsonStr.concat(field: value)
*/


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

    bool setConfig(const String& json);
    bool initializeConfig();
    bool saveConfig();
    void printConfigFile();
    bool deleteConfigFile();

private:
    bool _verbose = true;
    char _configPath[MAX_PATH_LENGTH];

    bool _readConfigFile();
    void _printConfigDoc();

    void _print(String str);
    void _println(String str);
};

#include "ConfigService.hpp"
