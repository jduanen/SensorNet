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

#define APPL_NAME           "WebServicesTest"
#define APPL_VERSION        "1.0.0"
#define WEB_SERVER_PORT     80
//#define USER_NAME       "name"
//#define PASSWD          "passwd"

#define CONFIG_PATH         "/config.json"
#define COMMON_PAGE_PATH    "/"
#define APPL_PAGE_PATH      "/app"

#define FW_UPDATE           (1 << 0)
#define CONFIG_FILE         (1 << 1)
#define COMMON_GUI          (1 << 2)
#define APPL_GUI            (1 << 3)

//#define TEST_MODE           FW_UPDATE
//#define TEST_MODE           FW_UPDATE | CONFIG_FILE
//#define TEST_MODE           FW_UPDATE | COMMON_GUI
//#define TEST_MODE           FW_UPDATE | APPL_GUI
//#define TEST_MODE           FW_UPDATE | CONFIG_FILE | COMMON_GUI
//#define TEST_MODE           FW_UPDATE | CONFIG_FILE | COMMON_GUI | APPL_GUI


const char* ssid = WLAN_SSID;
const char* password = WLAN_PASS;

WebServices webSvcs(APPL_NAME, WEB_SERVER_PORT);

int testMode = FW_UPDATE | COMMON_GUI;
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

  String confPath = "";
  String commonPagePath = "";
  String applPagePath = "";

  if ((testMode & CONFIG_FILE) == CONFIG_FILE) {
    confPath = CONFIG_PATH;
  }
  if ((testMode & COMMON_GUI) == COMMON_GUI) {
    commonPagePath = COMMON_PAGE_PATH;
  }
  if ((testMode & APPL_GUI) == APPL_GUI) {
    applPagePath = APPL_PAGE_PATH;
  }

  //// TODO register appl page
  webSvcs.setup(confPath, commonPagePath, applPagePath);
}

void loop() {
    webSvcs.run();
    if ((loopCnt % 100000) == 0) {
        Serial.println("loop: " + String(loopCnt));
    }
    loopCnt++;
};
