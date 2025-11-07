/***************************************************************************
 *
 * Wifi Support Utilities Library
 * 
 ***************************************************************************/

#include "WiFiUtilities.h"


#define NUM_ITEMS(arr)  ((unsigned int)(sizeof(arr) / sizeof(arr[0])))
#define NUM_WIFI_MODES  NUM_ITEMS(wifiModes)


WiFiMode_t  wifiMode;

IPAddress   softIPA;

const char wifiModes[][4] = {"OFF", "STA", "A_P", "APS"};


void (*resetFunc)(void) = 0;

String rot47(String str) {
    String outStr = "";
    char oldChr, newChr;

    for (int i = 0; (i < str.length()); i++) {
        oldChr = str.charAt(i);
        if ((oldChr >= '!') && (oldChr <= 'O')) {
            newChr = ((oldChr + 47) % 127);
        } else {
            if ((oldChr >= 'P') && (oldChr <= '~')) {
                newChr = ((oldChr - 47) % 127);
            } else {
                newChr = oldChr;
            }
        }
        outStr.concat(newChr);
    }
    return(outStr);
}

String wiFiStatusToString(wl_status_t status) {
    String s = "WIFI_UNKNOWN: " + String(status);
    switch (status) {
        case WL_NO_SHIELD: s = "WIFI_NO_SHIELD";
        case WL_IDLE_STATUS: s = "WIFI_IDLE_STATUS";
        case WL_NO_SSID_AVAIL: s = "WIFI_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED: s = "WIFI_SCAN_COMPLETED";
        case WL_CONNECTED: s = "WIFI_CONNECTED";
        case WL_CONNECT_FAILED: s = "WIFI_CONNECT_FAILED";
        case WL_CONNECTION_LOST: s = "WIFI_CONNECTION_LOST";
        case WL_DISCONNECTED: s = "WIFI_DISCONNECTED";
    }
    return(s);
}

void setWiFiMode(uint8_t mode) {
    if (mode >= NUM_WIFI_MODES) {
        Serial.println("ERROR: invalid WiFi mode -- " + String(mode));
    } else {
        wifiMode = (WiFiMode_t)mode;
    }
}

String getWiFiMode() {
    return String(wifiModes[wifiMode]);
}

void wiFiConnect(const String& staSSID, const String& passwd) {
    wiFiConnect(staSSID, passwd, "");
}

void wiFiConnect(const String& staSSID, const String& passwd, const String& apSSID) {
    wifiMode = WIFI_STA;
    WiFi.mode(wifiMode);
    WiFi.begin(staSSID, passwd);

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi.." + wiFiStatusToString(WiFi.status()));
        if (i++ > MAX_WIFI_RETRIES) {
            Serial.print("Too many attempts to connect to WiFi, ");
            if (apSSID == "") {
                Serial.println("restarting...");
                resetFunc();
            }
            Serial.println("switching to AP mode...");
            wifiMode = WIFI_AP;
            WiFi.mode(wifiMode);
            WiFi.softAP(apSSID);
            break;
        }

        //// FIXME figure out how to trigger this
        if (false) {
            if (i++ > MAX_WIFI_RETRIES) {
                Serial.println("Using fallback WiFi parameters");
                ////WiFi.begin(WLAN_SSID, rot47(WLAN_PASS));
                delay(1000);
                i = 0;
            }
        }
    }

    if (wifiMode == WIFI_STA) {
        Serial.println("\nWiFi Station Mode");
        Serial.println("Connected to " + WiFi.SSID());
        Serial.println("IP address: " + WiFi.localIP().toString());
    }
    if (wifiMode == WIFI_AP) {
        Serial.println("\nWiFi Access Point Mode");
        Serial.println("AP SSID: " + apSSID);
        softIPA = WiFi.softAPIP();
        Serial.println("AP IP Address: " + softIPA.toString());
    }
    Serial.println("RSSI: " + String(WiFi.RSSI()));
}
