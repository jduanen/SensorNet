#include "wifi.h"
#include "mqtt.h"

const char *ssid = WLAN_SSID;
const char *password =  WLAN_PASS;
const char *mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
String RAD_TOPIC = "/sensors/radiation/";
const int MAX_MSG_LEN = 128;
byte mac[6];
String macAddr;
const int MAX_TOPIC_LEN = 80;
char radData[MAX_TOPIC_LEN];
char radCmd[MAX_TOPIC_LEN];

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  delay(30);
  Serial.begin(9600); /* 115200 */
  Serial.println("Radiation");

  WiFi.begin(ssid, password);
  Serial.println("Starting WIFI");
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi.." + String(WiFi.status()));
  }

  WiFi.macAddress(mac);
  macAddr = String(mac[0], HEX) + ":" + 
            String(mac[1], HEX) + ":" +
            String(mac[2], HEX) + ":" +
            String(mac[3], HEX) + ":" +
            String(mac[4], HEX) + ":" +
            String(mac[5], HEX);
  Serial.println("Connected to the WiFi network: " + macAddr);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.println("ERROR: failed with state " + client.state());
      delay(2000);
    }
  }

  /* TODO: add unique id -- e.g., MAC address */
  String rad = RAD_TOPIC + macAddr;
  String radStr;
  radStr = rad + "/data";
  radStr.toCharArray(radData, MAX_TOPIC_LEN);
  radStr = rad + "/cmd";
  radStr.toCharArray(radCmd, MAX_TOPIC_LEN);
  client.publish(radData, "ESP8266 Startup");
  client.subscribe(radCmd);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived in topic: " + String(topic));

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
}

void loop() {
  String inMsg;
  char buf[MAX_MSG_LEN];

  client.loop();

  if (Serial.available() > 0) {
    inMsg = Serial.readStringUntil('\n');
    Serial.println("> " + inMsg);
    inMsg.toCharArray(buf, MAX_MSG_LEN);
    buf[strlen(buf) - 1] = '\0';
    client.publish(radData, buf);
  }    
}
