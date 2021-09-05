/*
* Library that handles serial connections, WiFi, and MQTT for ESP8266-12e modules
*/

#include "Arduino.h"
#include "SensorNet.h"


SensorNet::SensorNet() {
    consolePtr = NULL;
}

SensorNet::SensorNet(String name) {
    consolePtr = NULL;
    appName = name;
}

SensorNet::SensorNet(String name, String version) {
    consolePtr = NULL;
    appName = name;
    appVersion = version;
}

SensorNet::SensorNet(String name, String version, String schema) {
    consolePtr = NULL;
    appName = name;
    appVersion = version;
    reportSchema = schema;
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

// Wait for input on console (if one is enabled)
void SensorNet::consoleWaitForInput() {
    if (consolePtr != NULL) {
        while (!consolePtr->available());
        while (consolePtr->available()) Serial.read();
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
        _ipAddr,
        WiFi.RSSI()
    };
    return status;
}

// Setup connection to MQTT server and make initial connection
void SensorNet::mqttSetup(String server, int port, String prefix) {
    //// TODO assert that WiFi.status() == WL_CONNECTED
    server.toCharArray(mqttServer, BUF_SIZE);
    mqttPort = port;
    mqttClient.setServer(mqttServer, mqttPort);
//    mqttClient.setSocketTimeout(?);
//    mqttClient.setKeepalive(?);

    baseTopic = prefix + "/" + _macAddr;
    String t = baseTopic + "/data";
    t.toCharArray(topics[DATA], MAX_MQTT_TOPIC_LEN);
    t = baseTopic + "/cmd";
    t.toCharArray(topics[COMMAND], MAX_MQTT_TOPIC_LEN);
    t = baseTopic + "/response";
    t.toCharArray(topics[RESPONSE], MAX_MQTT_TOPIC_LEN);
    t = baseTopic + "/error";
    t.toCharArray(topics[ERROR], MAX_MQTT_TOPIC_LEN);

    mqttRun();
    String startupMsg = "Startup,ESP8266," +
                        appName + "," +
                        appVersion + "," +
                        reportSchema + "," +
                        wifiState().rssi;
    startupMsg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
    mqttClient.publish(topics[COMMAND], pubMsg);
}

// Connect to the MQTT server
bool SensorNet::mqttRun() {
    //// TODO assert that MQTT has been set up
    bool result = true;
    if (!mqttClient.connected()) {
        consolePrint("Connecting to MQTT...");
        if (mqttClient.connect(_clientName)) {
            consolePrintln("connected");
        } else {
            String msg = "ERROR: failed with state " + mqttClient.state();
            consolePrintln(msg);
            msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
            mqttClient.publish(topics[ERROR], pubMsg);
            result = false;
        }
    }
    mqttClient.loop();
    return result;
}

// Publish message to the given subtopic of the defined topic
bool SensorNet::mqttPub(pubType type, String msg) {
    //// TODO assert that MQTT has been set up
    bool result = true;
    if ((type < 0) || (type >= NUM_SUB_TOPICS)) {
        msg = "ERROR: bad sub-topic: " + String(type);
        type = ERROR;
        consolePrintln(msg);
        result = false;
    }
    msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
    mqttClient.publish(topics[type], pubMsg);
    return result;
}

// Add a callback function to the subscribed "cmd" topic
bool SensorNet::mqttSub(pubType type, void (*callback)(char *topic, byte *payload, unsigned int length)) {
    //// TODO assert that MQTT has been set up
    bool result = true;
    mqttClient.setCallback(callback);
    if (mqttClient.subscribe(topics[COMMAND]) == false) {
        String msg = "ERROR: failed to subscribe to topic";
        consolePrintln(msg);
        msg.toCharArray(pubMsg, MAX_MQTT_PUB_MSG_LEN);
        mqttClient.publish(topics[ERROR], pubMsg);
        result = false;
    } else {
        consolePrintln(String("Subscribed to: ") + String(topics[COMMAND]));
    }
    return result;
}

SensorNet::MQTT_STATE SensorNet::mqttState() {
    SensorNet::MQTT_STATE state = {
        String(mqttServer),
        mqttPort,
        String(baseTopic)
    };
    return state;
}
