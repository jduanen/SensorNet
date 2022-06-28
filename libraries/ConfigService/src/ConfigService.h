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
#define CS_USE_LITTLEFS     true
#include <ConfigStorage.h>


#define VERBOSE                 1
#define LIB_VERSION             "1.0"

#define DEF_CONFIG_PATH         "/config.json"
#define JSON_OBJ_SIZE           512


class ConfigService {
public:
    String libVersion = LIB_VERSION;

	ConfigService(const String& configPath=DEF_CONFIG_PATH);
    void initializeConfig(const String& configPath);
    void updateConfig(StaticJsonDocument<JSON_OBJ_SIZE> confDoc);

private:
	ConfigStorage _cs;
	StaticJsonDocument<JSON_OBJ_SIZE> _configJson;

    void _saveConfig();
}


#endif /*CONFIG_SERVICE_H*/
