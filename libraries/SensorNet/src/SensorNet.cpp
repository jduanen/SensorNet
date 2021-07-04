#include "Arduino.h"
#include "SensorNet.h"


SensorNet::SensorNet() {
	consolePtr = NULL;
}

// Initialize a serial port
//// TODO figure out default parameters
void SensorNet::serialStart(HardwareSerial *portPtr, uint16 baud, bool console) {
	portPtr->begin(baud);
	portPtr->println("INIT SERIAL");
	if (console) {
		consolePtr = portPtr;
	}
}

// Initialize the WiFi connection
void SensorNet::wifiStart(String ssid, String password) {
  WiFi.begin(ssid, password);
	condPrint("Starting WIFI...");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    condPrintln("Connecting to WiFi.." + String(WiFi.status()));
  }

  _ipAddr = WiFi.localIP();
  WiFi.macAddress(_mac);
  _macAddr = String(_mac[0], HEX) + ":" + 
             String(_mac[1], HEX) + ":" +
             String(_mac[2], HEX) + ":" +
             String(_mac[3], HEX) + ":" +
             String(_mac[4], HEX) + ":" +
             String(_mac[5], HEX);
  condPrintln("Connected to the WiFi network: " + _macAddr + " @ " + _ipAddr.toString());
}

// Get state of WiFi connection
SensorNet::WIFI_STATE SensorNet::wifiState() {
	SensorNet::WIFI_STATE status = {
		WiFi.status(),
		_macAddr,
		_ipAddr
	};
	return status;
}

// Make connection to an MQTT server
void SensorNet::mqttStart(String server, int port, String prefix) {
	server.toCharArray(mqttServer, BUF_SIZE);
	mqttPort = port;
  mqttClient.setServer(mqttServer, mqttPort);

  while (!mqttClient.connected()) {
    condPrint("Connecting to MQTT...");
    if (mqttClient.connect("ESP8266mqttClient")) {
      condPrintln("connected");
    } else {
      condPrintln("ERROR: failed with state " + mqttClient.state());
      delay(2000);
    }
  }

  String topic = prefix + "/" + _macAddr + "/data";
  topic.toCharArray(dataTopic, MAX_MQTT_TOPIC_LEN);
  condPrintln(dataTopic);
  mqttClient.publish(dataTopic, "ESP8266 Startup");
}

// Publish message to the defined topic
void SensorNet::mqttPub(String msg) {
	msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
	mqttClient.publish(dataTopic, pubMsg);
}


/*
// Subscribe to a topic and give a callback handler
void SensorNet::mqttSub(void *callback()){
  mqttClient.setCallback(callback);
  String rad = radTopic + macAddr;
  radStr = rad + "/cmd";
  radStr.toCharArray(radCmd, MAX_TOPIC_LEN);}
  mqttClient.subscribe(radCmd);
*/

// Conditional print string
void SensorNet::condPrint(String str) {
  if (consolePtr) {
  	consolePtr->print(str);
	}
}

// Conditional println string
void SensorNet::condPrintln(String str) {
  if (consolePtr) {
  	consolePtr->println(str);
	}
}
