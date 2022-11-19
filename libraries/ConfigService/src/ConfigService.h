/***************************************************************************
 *
 * Library that handles application configuration state
 * 
 * N.B. config files are assumed to be <1KB, so use StaticJsonDocument
 *       which stores things on the stack.
 *
 ***************************************************************************/

#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "LfsUtilities.h"


#define MAX_PATH_LENGTH         32

#define INIT_CONFIG_PTR(csPtr, field, value)        if (!csPtr->validEntry(field)) {(*(csPtr->doc))[field] = value;}
#define SET_CONFIG_PTR(csPtr, field, value)         (*(csPtr->doc))[field] = value
#define GET_CONFIG_PTR(value, csPtr, field, typ)    value = (*(csPtr->doc))[field].as<typ>()

#define INIT_CONFIG(csObj, field, value)        if (!csObj.validEntry(field)) {(*(csObj.doc))[field] = value;}
#define SET_CONFIG(csObj, field, value)         (*(csObj.doc))[field] = value
#define GET_CONFIG(value, csObj, field, typ)    value = (*(csObj.doc))[field].as<typ>()


class ConfigService {
public:
    String libVersion = "2.0";
    DynamicJsonDocument *doc;

    ConfigService(uint32_t docSize, const String& configFilePath);
    ~ConfigService();
    static ConfigService &getInstance();

    void format();
    void listFiles(const String& path);
    bool initializeConfig();
    bool saveConfig();
    void printConfig();
    bool deleteConfig();
    bool validEntry(const String& key);

private:
    bool _verbose = true;
    char _configPath[MAX_PATH_LENGTH];

    bool _readConfig();
    void _printConfigDoc();

    void _print(String str);
    void _println(String str);
};

#endif /*CONFIG_SERVICE_H*/
