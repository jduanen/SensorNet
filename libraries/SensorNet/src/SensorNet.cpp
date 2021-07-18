#include "Arduino.h"
#include "SensorNet.h"


SensorNet::SensorNet() {
	consolePtr = NULL;
}

SensorNet::SensorNet(String name) {
	consolePtr = NULL;
	appName = name;
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

// Print on console (if one is enabled)
void SensorNet::consolePrint(String str) {
  if (consolePtr != NULL) {
  	consolePtr->print(str);
	}
}

// Println on console (if one is enabled)
void SensorNet::consolePrintln(String str) {
  if (consolePtr != NULL) {
  	consolePtr->println(str);
	}
}

// Initialize the WiFi connection
void SensorNet::wifiStart(String ssid, String password) {
  WiFi.begin(ssid, password);
	consolePrint("Starting WIFI...");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    consolePrintln("Connecting to WiFi.." + String(WiFi.status()));
  }

  _ipAddr = WiFi.localIP();
  WiFi.macAddress(_mac);
  _macAddr = String(_mac[0], HEX) + ":" + 
             String(_mac[1], HEX) + ":" +
             String(_mac[2], HEX) + ":" +
             String(_mac[3], HEX) + ":" +
             String(_mac[4], HEX) + ":" +
             String(_mac[5], HEX);
	String(appName + "_" + _macAddr).toCharArray(_clientName, BUF_SIZE);

  consolePrintln("Connected to the WiFi network: " + _macAddr + " @ " + _ipAddr.toString());
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

// Setup connection to MQTT server and make initial connection
void SensorNet::mqttSetup(String server, int port, String prefix) {
	//// TODO assert that WiFi.status() == WL_CONNECTED
	server.toCharArray(mqttServer, BUF_SIZE);
	mqttPort = port;
  mqttClient.setServer(mqttServer, mqttPort);
//  mqttClient.setSocketTimeout(?);
//  mqttClient.setKeepalive(?);

//  String topic = prefix + "/" + _macAddr + "/cmd";
  String topic = prefix + "/#";
  topic.toCharArray(cmdTopic, MAX_MQTT_TOPIC_LEN);

  topic = prefix + "/" + _macAddr + "/data";
  topic.toCharArray(dataTopic, MAX_MQTT_TOPIC_LEN);
  mqttClient.publish(dataTopic, "ESP8266 Startup");
}

// Connect to the MQTT server
void SensorNet::mqttRun() {
	//// TODO assert that MQTT has been set up
  while (!mqttClient.connected()) {
    consolePrint("Connecting to MQTT...");
    if (mqttClient.connect(_clientName)) {
      consolePrintln("connected");
    } else {
      consolePrintln("ERROR: failed with state " + mqttClient.state());
      delay(2000);
    }
  }
  mqttClient.loop();
}

// Publish message to the defined topic
void SensorNet::mqttPub(String msg) {
	msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
	mqttClient.publish(dataTopic, pubMsg);
//	consolePrintln("ZZZZ");
//	mqttClient.publish(dataTopic, "YYYY");
}

// Subscribe to a topic and give a callback handler
void SensorNet::mqttSub(void (*callback)(char *, byte *, unsigned int)) {
  mqttClient.setCallback(callback);
  if (mqttClient.subscribe(cmdTopic) == false) {
  	consolePrint("ERROR: failed to subscribe to topic -- ");
  	consolePrintln(cmdTopic);
  	//// TODO handle error
  } else {
	  consolePrintln(String("Subscribed to: ") + String(cmdTopic));
	}
}

SensorNet::MQTT_STATE SensorNet::mqttState() {
	SensorNet::MQTT_STATE state = {
		String(mqttServer),
		mqttPort,
		String(dataTopic),
		String(cmdTopic)
	};
	return state;
}
