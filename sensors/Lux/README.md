# Light Sensor

**TBD**

Battery voltage monitoring (optional)
Since the ESP32-C3 lacks native battery monitoring, use a voltage divider (two 200kΩ resistors) to halve the battery voltage and connect to A0:

float Vbattf = 2 * analogReadMilliVolts(A0) / 1000.0; // Battery voltage in volts

Links:
  * Cad models
    - https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/6498%20MAX44009%20Light%20Sensor
  * ?
    - https://community.home-assistant.io/t/esphome-custom-sensor-bq27441/483177

  * ?
    - https://search.brave.com/search?q=esphome+BQ27441-G1A&source=web&summary=1&conversation=093c742a255f4e6c0879d79ca2c9a547698d

======================================================


esphome:
  name: battery-monitor
  includes:
    - bq27441_custom.h
  libraries:
    - "Wire"
    - "SparkFun BQ27441 LiPo Fuel Gauge Arduino Library"

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

sensor:
  - platform: custom
    lambda: |-
      auto bq_sensor = new MyBQ27441();
      App.register_component(bq_sensor);
      return {bq_sensor};
    sensors:
      - name: "Battery Level"
        unit_of_measurement: "%"
        accuracy_decimals: 0   


#include "esphome.h"
#include "SparkFunBQ27441.h"

class MyBQ27441 : public PollingComponent, public Sensor {
public:
  MyBQ27441() : PollingComponent(60000) {} // Poll every 60 seconds

  float get_setup_priority() const override { return esphome::setup_priority::HARDWARE; }

  void setup() override {
    lipo.begin();
    lipo.setCapacity(2000); // Adjust to your battery's mAh
  }

  void update() override {
    unsigned int soc = lipo.soc();
    publish_state(soc);
  }

private:
  SFE_BQ27441 lipo;
};   
