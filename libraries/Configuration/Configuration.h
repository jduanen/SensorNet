/*
* Library that handles application configuration
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#define CS_USE_LITTLEFS     true
#include <ConfigStorage.h>


#define CONFIG_PATH         "/config.json"


ConfigStorage cs(CONFIG_PATH);

StaticJsonDocument<512> config;


#endif /*CONFIGURATION_H*/
