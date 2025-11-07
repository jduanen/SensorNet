/*
* Library that handles serial connections, WiFi, and MQTT for ESP8266-12e modules
*/

#ifndef SENSOR_NET_H
#define SENSOR_NET_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>


#define NUM_SERIALS             2
//#define MAX_MQTT_PUB_MSG_LEN    180
//#define MAX_MQTT_TOPIC_LEN      128
//#define BUF_SIZE                64
#define NUM_SUB_TOPICS          5
#define DEF_REPORT_INTERVAL     60000  // one report every minute
#define LIB_VERSION             "2.0"

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))


typedef void (callback)(char *topic, byte *payload, unsigned int length);

class SensorNet {
public:
    String appName = "n/a";
    String appVersion = "n/a";
    String reportSchema = "n/a";
    HardwareSerial *consolePtr;
    String libVersion = LIB_VERSION;

    unsigned long lastReport = 0;
    unsigned int reportInterval = DEF_REPORT_INTERVAL;

    // pub msg types -- 1:1 correspondance to sub-topics
    typedef char pubType;
    static const pubType DATA = 0;
    static const pubType COMMAND = 1;
    static const pubType RESPONSE = 2;
    static const pubType ERROR = 3;
    static const pubType STARTUP = 4;

    struct WIFI_STATE {
        wl_status_t state;
        String macAddr;
        IPAddress ipAddr;
        long rssi;
    };

    struct MQTT_STATE {
        String server;
        int port;
        String baseTopic;
    };

    typedef struct {
        String cmd;
        String val;
        bool handled;
    } callbackMessage;

    SensorNet(const String& name="", const String& version="", const String& schema="", const String& cmdNames="");

    void(* systemReset)(void) = 0;

    void serialStart(HardwareSerial *portPtr, uint16_t baud, bool console);
    void consolePrint(const String& str);
    void consolePrintln(const String& str);
    void consoleWaitForInput();

    void wifiStart(const String& ssid, const String& password);
    WIFI_STATE wifiState();

    void mqttSetup(const String& server, uint16_t port, const String& prefix, callback *cb=nullptr);
    bool mqttRun();
    bool mqttPub(pubType type, const String& msg);
    MQTT_STATE mqttState();

    callbackMessage baseCallback(char* topic, byte* payload, unsigned int length);

    String commandNames();

private:
    byte        _mac[6];
    String      _macAddr;
    IPAddress   _ipAddr;
    String      _clientName;
    String      _mqttServer;
    uint16_t    _mqttPort;
    String      _baseTopic;
    String      _topics[NUM_SUB_TOPICS];
    String      _commandNames = "Rate,Reset,RSSI,Schema,Version";

    WiFiClient _espClient;
    PubSubClient _mqttClient = PubSubClient(_espClient);  //// FIXME dynamically allocate this

    void _topicSubscribe(const String& topic);
};

#endif /*SENSOR_NET_H*/
