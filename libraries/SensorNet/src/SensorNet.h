/*
* Library that handles serial connections, WiFi, and MQTT for ESP8266-12e modules
*/

#ifndef SENSOR_NET_H
#define SENSOR_NET_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define NUM_SERIALS             2
#define MAX_MQTT_PUB_MSG_LEN    180
#define MAX_MQTT_TOPIC_LEN      128
#define BUF_SIZE                64
#define NUM_SUB_TOPICS			5
#define DEF_REPORT_INTERVAL     60000  // one report every minute
#define LIB_VERSION             "1.1"

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))


typedef void (callback)(char *topic, byte *payload, unsigned int length);


class SensorNet {
    public:
        String appName = "n/a";
        String appVersion = "n/a";
        String reportSchema = "n/a";
        HardwareSerial *consolePtr;

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

        SensorNet();
        SensorNet(String name);
        SensorNet(String name, String version);
        SensorNet(String name, String version, String schema);

	    void(* systemReset)(void) = 0;

        void serialStart(HardwareSerial *portPtr, uint16 baud, bool console);
        void consolePrint(String str);
        void consolePrintln(String str);
        void consoleWaitForInput();

        void wifiStart(String ssid, String password);
        WIFI_STATE wifiState();

        void mqttSetup(String server, int port, String prefix);
        void mqttSetup(String server, int port, String prefix, callback *cb);
        bool mqttRun();
        bool mqttPub(pubType type, String msg);
        MQTT_STATE mqttState();

        callbackMessage baseCallback(char* topic, byte* payload, unsigned int length);

    private:
        byte _mac[6];
        String _macAddr;
        IPAddress _ipAddr;
        char _clientName[BUF_SIZE];

        char mqttServer[BUF_SIZE];
        int mqttPort;
        String baseTopic;
        char pubMsg[MAX_MQTT_PUB_MSG_LEN];

        WiFiClient espClient;
        PubSubClient mqttClient = PubSubClient(espClient);

        char topics[NUM_SUB_TOPICS][MAX_MQTT_TOPIC_LEN];

        void topicSubscribe(String topic);
};

#endif /*SENSOR_NET_H*/
