/*
* Library that handles serial connections, WiFi, and MQTT for ESP8266-12e modules
*/

#ifndef SENSOR_NET_H
#define SENSOR_NET_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define NUM_SERIALS      	 		2
#define MAX_MQTT_PUB_MSG_LEN	128
#define MAX_MQTT_TOPIC_LEN		80
#define BUF_SIZE              64

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

class SensorNet {
	public:
		String appName = "UNKNOWN";
		HardwareSerial *consolePtr;

		struct WIFI_STATE {
			wl_status_t state;
			String macAddr;
			IPAddress ipAddr;
		};

		struct MQTT_STATE {
			String server;
			int port;
			String dataTopic;
			String cmdTopic;
		};

	  SensorNet();
	  SensorNet(String);

	  void serialStart(HardwareSerial *portPtr, uint16 baud, bool console);
	  void consolePrint(String str);
	  void consolePrintln(String str);

    void wifiStart(String ssid, String password);
	  WIFI_STATE wifiState();

	  void mqttSetup(String server, int port, String prefix);
	  void mqttRun();
	  void mqttPub(String msg);
	  void mqttSub(void (*callback)(char *, byte *, unsigned int));
		MQTT_STATE mqttState();

	private:
  	byte _mac[6];
  	String _macAddr;
  	IPAddress _ipAddr;
		char _clientName[BUF_SIZE];

  	char mqttServer[BUF_SIZE];
	  int mqttPort;
    char dataTopic[MAX_MQTT_TOPIC_LEN];
    char cmdTopic[MAX_MQTT_TOPIC_LEN];
		char pubMsg[MAX_MQTT_PUB_MSG_LEN];

 		WiFiClient espClient;
  	PubSubClient mqttClient = PubSubClient(espClient);
};

#endif /*SENSOR_NET_H*/
