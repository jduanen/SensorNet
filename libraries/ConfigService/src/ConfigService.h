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


#define JSON_OBJ_SIZE           1024
#define MAX_PATH_LENGTH         80

//// FIXME make the size a constructor argument
typedef StaticJsonDocument<JSON_OBJ_SIZE> ConfigJsonDoc;


class ConfigService {
public:
    String libVersion = "1.1";
    ConfigJsonDoc configJsonDoc;

    ConfigService();
    ~ConfigService();
    static ConfigService &getInstance();

    bool open(const String& configPath);
    void close();
    void format();
    void listFiles(const String& path);
    bool initializeConfig();
    bool saveConfig();
    void printConfig();
    bool deleteConfig();

private:
    bool _verbose = true;
    char _configPath[MAX_PATH_LENGTH];

    bool _readConfig();

    void _displayConfigDoc();

    void _print(String str);
    void _println(String str);
};


extern ConfigService &cs;


#endif /*CONFIG_SERVICE_H*/
