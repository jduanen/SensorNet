/***************************************************************************
 *
 * ESP Arduino WiFi Utilities Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "WiFiUtilities.h"
#include "wifi.h"


#define VERBOSE         1
#define APP_NAME        "WiFiUtilitiesTest"
#define APP_VERSION     "1.0.0"


const char *ssid = WLAN_SSID;
const char *password = WLAN_PASS;

uint32_t    loopCnt = 0;


void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    delay(1000);
    Serial.print(APP_NAME); Serial.println(": BEGIN");
    wiFiConnect(ssid, password);
    Serial.print(APP_NAME); Serial.println(": READY");
}

void loop() {
    if ((loopCnt++ % 100) == 0) {
        Serial.println("loopCnt: " + String(loopCnt));
    }
    delay(100);
};
