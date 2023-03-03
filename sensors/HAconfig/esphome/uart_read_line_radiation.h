#include "esphome.h"

class UartReadLineRadiation : public Component, public UARTDevice, public Sensor {
  public:
    UartReadLineRadiation(UARTComponent *parent) : UARTDevice(parent) {}
    Sensor *countsPerMin_sensor = new Sensor();
    Sensor *uSvPerHr_sensor = new Sensor();
    Sensor *volts_sensor = new Sensor();

    void setup() override {
        // nothing to do here
    }

    int readline(int readch, char *buffer, int len) {
        static int pos = 0;
        int rpos;

        if (readch > 0) {
            switch (readch) {
                case '\n': // Ignore new-lines
                    break;
                case '\r': // Return on CR
                    rpos = pos;
                    pos = 0;  // Reset position index ready for next time
                return rpos;
            default:
                if (pos < (len - 1)) {
                    buffer[pos++] = readch;
                    buffer[pos] = 0;
                }
            }
        }
        return -1;    // No end of line has been found
    }

    void loop() override {
        const int max_line_length = 80;
        static char buffer[max_line_length];

        while (available()) {
            if (readline(read(), buffer, max_line_length) > 0) {
                int countsPerMin;
                float uSvPerHr, volts;
                int n = sscanf(buffer, "%d,%f,%f", &countsPerMin, &uSvPerHr, &volts);
                if (n != 3) {
                    ESP_LOGE("custom", "Failed to read from GK sensor board");
                }
                /*
                char buf[64];  //// FIXME
                sprintf(buf, "{\"cpm\": %d, \"uSvPerHr\": %.4f, \"volts\": %.4f}", countsPerMin, uSvPerHr, volts);
                publish_state(buf);
                */
                countsPerMin_sensor->publish_state(countsPerMin);
                uSvPerHr_sensor->publish_state(uSvPerHr);
                volts_sensor->publish_state(volts);
            }
        }
    }
};
