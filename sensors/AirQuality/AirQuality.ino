#include "PMS.h"
#include "wifi.h"
#include "mqtt.h"
#include <ESP8266WiFi.h>

const char *ssid = WLAN_SSID;
const char *password =  WLAN_PASS;
const char *mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
String AIR_TOPIC = "/sensors/airQuality/";
const int MAX_MSG_LEN = 128;
byte mac[6];
String macAddr;
IPAddress ipAddr;
const int MAX_TOPIC_LEN = 80;
char airData[MAX_TOPIC_LEN];
char airCmd[MAX_TOPIC_LEN];
const int VERBOSE = 0;

WiFiClient espClient;
PubSubClient client(espClient);

PMS pms(Serial);
PMS::DATA data;

void setup()
{
  const int MAX_NAME_LEN = 64;
  char clientName[MAX_NAME_LEN];
  String name = "AirQuality";

  Serial.begin(9600);   // GPIO1, GPIO3 (TX/RX pin on ESP-12E Development Board)
  Serial1.begin(9600);  // GPIO2 (D4 pin on ESP-12E Development Board)
  Serial1.println(name);

  WiFi.begin(ssid, password);
  Serial1.println("Starting WIFI");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.println("Connecting to WiFi.." + String(WiFi.status()));
  }

  ipAddr = WiFi.localIP();
  WiFi.macAddress(mac);
  macAddr = String(mac[0], HEX) + ":" + 
            String(mac[1], HEX) + ":" +
            String(mac[2], HEX) + ":" +
            String(mac[3], HEX) + ":" +
            String(mac[4], HEX) + ":" +
            String(mac[5], HEX);
  name += "_" + macAddr;
  name.toCharArray(clientName, MAX_NAME_LEN);
  Serial1.println("Connected to the WiFi network: " + macAddr + " @ " + ipAddr.toString());

  client.setServer(mqttServer, mqttPort);
  client.setSocketTimeout(90);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial1.print("Connecting to MQTT...");
    if (client.connect(clientName)) {
      Serial1.println("connected");
    } else {
      Serial1.println("ERROR: failed with state " + client.state());
      delay(2000);
    }
  }

  String air = AIR_TOPIC + macAddr;
  String airStr;
  airStr = air + "/data";
  airStr.toCharArray(airData, MAX_TOPIC_LEN);
  airStr = air + "/cmd";
  airStr.toCharArray(airCmd, MAX_TOPIC_LEN);
  client.publish(airData, "ESP8266 Startup");
  Serial1.println("airData: " + String(airData));
  client.subscribe(airCmd);
  Serial1.println("airCmd: " + String(airCmd));

  pms.passiveMode();    // Switch to passive mode
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial1.println("Message arrived in topic: " + String(topic));

  Serial1.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial1.print((char)payload[i]);
  }

  Serial1.println();
  Serial1.println("-----------------------");
}

void loop()
{
  String msg;
  char buf[MAX_MSG_LEN];

  Serial1.println("Waking up, wait 30 seconds for stable readings...");
  pms.wakeUp();
  delay(30000);

  Serial1.println("Send read request...");
  pms.requestRead();

  Serial1.println("Wait max. 1 second for read...");
  if (pms.readUntil(data)) {
    if (VERBOSE) {
      Serial1.print("PM 1.0 (ug/m3): ");
      Serial1.println(data.PM_AE_UG_1_0);

      Serial1.print("PM 2.5 (ug/m3): ");
      Serial1.println(data.PM_AE_UG_2_5);

      Serial1.print("PM 10.0 (ug/m3): ");
      Serial1.println(data.PM_AE_UG_10_0);
    }

    msg = String(data.PM_AE_UG_1_0) + "," + 
          String(data.PM_AE_UG_2_5) + "," + 
          String(data.PM_AE_UG_10_0);
    msg.toCharArray(buf, MAX_MSG_LEN);
    client.publish(airData, buf);
    Serial1.println(buf);
  } else {
    Serial1.println("No data.");
  }

  Serial1.println("Going to sleep for 30 seconds.");
  pms.sleep();
  delay(30000);
}
