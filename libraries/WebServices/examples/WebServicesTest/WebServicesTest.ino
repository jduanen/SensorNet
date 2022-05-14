/***************************************************************************
 *
 * SensorNet WebServices library test
 *
 * Notes:
 *   - ?
 *
 ***************************************************************************/

#include <WebServices.h>
#include "wifi.h"


#define VERBOSE             1

#define WEB_SERVER_PORT     80
//#define USER_NAME       "name"
//#define PASSWD          "passwd"


const char* ssid = WLAN_SSID;
const char* password = WLAN_PASS;

WebServices webSvcs(WEB_SERVER_PORT);

int loopCnt = 0;

void setup() {
  delay(500);
  Serial.begin(19200);
  delay(500);
  Serial.println("\nBEGIN");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  webSvcs.setup("", "");
//  webSvcs.setup("/config.json", null);
//  webSvcs.setup(null, "/");
//  webSvcs.setup("/config.json", "/");
}

void loop() {
    webSvcs.run();
    if ((loopCnt % 100000) == 0) {
        Serial.println("loop: " + String(loopCnt));
    }
    loopCnt++;
};
