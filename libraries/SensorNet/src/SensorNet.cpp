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
	//// TODO assert that WiFi.status() == WL_CONNECTED
	char clientName[64];
	String(appName + "_" + _macAddr).toCharArray(clientName, 64);

	server.toCharArray(mqttServer, BUF_SIZE);
	mqttPort = port;
  mqttClient.setServer(mqttServer, mqttPort);

  while (!mqttClient.connected()) {
    condPrint("Connecting to MQTT...");
    if (mqttClient.connect(clientName)) {
      condPrintln("connected");
    } else {
      condPrintln("ERROR: failed with state " + mqttClient.state());
      delay(2000);
    }
  }

//  String topic = prefix + "/" + _macAddr + "/cmd";
  String topic = prefix + "/#";
  topic.toCharArray(cmdTopic, MAX_MQTT_TOPIC_LEN);

  topic = prefix + "/" + _macAddr + "/data";
  topic.toCharArray(dataTopic, MAX_MQTT_TOPIC_LEN);
  mqttClient.publish(dataTopic, "ESP8266 Startup");
}

// Publish message to the defined topic
void SensorNet::mqttPub(String msg) {
	msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
	mqttClient.publish(dataTopic, pubMsg);
//	condPrintln("ZZZZ");
//	mqttClient.publish(dataTopic, "YYYY");
}

// Subscribe to a topic and give a callback handler
void SensorNet::mqttSub(void (*callback)(char *, byte *, unsigned int)) {
  mqttClient.setCallback(callback);
  if (mqttClient.subscribe(cmdTopic) == false) {
  	condPrint("ERROR: failed to subscribe to topic -- ");
  	condPrintln(cmdTopic);
  	//// TODO handle error
  } else {
	  condPrintln(String("Subscribed to: ") + String(cmdTopic));
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
