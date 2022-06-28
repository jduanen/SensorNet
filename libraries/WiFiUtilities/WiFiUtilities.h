/***************************************************************************
 *
 * WiFi Support Utilities Library
 * 
 ***************************************************************************/

#ifndef WIFI_UTILITIES_H
#define WIFI_UTILITIES_H

#include <ESP8266WiFi.h>


#define MAX_WIFI_RETRIES    8 //64


String rot47(String str);

String wiFiStatusToString(wl_status_t status);

void setWiFiMode(uint8_t mode);

String getWiFiMode();

void wiFiConnect(String staSSID, String passwd, String apSSID);


#endif /*WIFI_UTILITIES_H*/
