/***************************************************************************
 *
 * Library that handles application configuration
 * 
 ***************************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#define CS_USE_LITTLEFS     true
#include <ConfigStorage.h>


#define VERBOSE                 1
#define LIB_VERSION             "1.0"

#define DEF_CONFIG_PATH         "/config.json"


class ConfigService {
public:
    String libVersion = LIB_VERSION;

	ConfigService(String configPath=DEF_CONFIG_PATH);

private:
	ConfigStorage _cs;

	StaticJsonDocument<512> _configJson;
}


#endif /*CONFIGURATION_H*/
