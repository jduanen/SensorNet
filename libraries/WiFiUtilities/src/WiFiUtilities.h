/***************************************************************************
 *
 * WiFi Support Utilities Library
 * 
 ***************************************************************************/

#ifndef WIFI_UTILITIES_H
#define WIFI_UTILITIES_H
#if !defined(ESP_PLATFORM)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif


#define MAX_WIFI_RETRIES    8 //64


String rot47(String str);

String wiFiStatusToString(wl_status_t status);

void setWiFiMode(uint8_t mode);

String getWiFiMode();

void wiFiConnect(const String& staSSID, const String& passwd);

void wiFiConnect(const String& staSSID, const String& passwd, const String& apSSID);


#endif /*WIFI_UTILITIES_H*/
