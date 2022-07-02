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
#include <LfsUtilities.h>


#ifndef VERBOSE
#define VERBOSE                 0
#endif

#define LIB_VERSION             "1.0"

#define JSON_OBJ_SIZE           512
#define MAX_PATH_LENGTH         80


typedef StaticJsonDocument<JSON_OBJ_SIZE> ConfigJsonDoc;


class ConfigService {
public:
    String libVersion = LIB_VERSION;
    ConfigJsonDoc configJsonDoc;

	ConfigService(const String& configPath);
    ~ConfigService();
    bool initializeConfig();
    bool saveConfig();
    void printConfig();
    bool deleteConfig();

private:
    char _configPath[MAX_PATH_LENGTH];

    bool _readConfig();

    void _displayConfigDoc();
};


#endif /*CONFIG_SERVICE_H*/
