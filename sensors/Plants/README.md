# Battery-Powered Plant Light and Moisture Sensor

**WIP**

## Hardware

### Sparkfun Triad Spectroscopy Sensor (AS7265x)

The sensor consists of three AS7265x sensors are combined alongside a visible, UV, and IR LEDs to illuminate and test various surfaces for light spectroscopy.
These three sensors are the AS72651 (UV), AS72652 (visible), and AS72653 (near-IR) sensors, which, taken together, cover the measurement of light from 410 nm to 940 nm in 18 individual bands.
The sensors' operating firmware modules are loaded from an on-board EEPROM at power up.

In addition to the light sensors, this board has a (5700K) white LED, a (405nm) UV LED, and a (875nm) IR LED, which are all controlled via software-defined drive currents.
There is also a red power LED and a blue board status LED on the sensor board.

The sensor board provides both I2C and serial UART interfaces (both of which operate with 3V3). The serial interface operates with AT commands at 115200 baud, and requires that you first disable the I2C interface by cutting a trace on the board.

### Battery Charging and Management

Battery voltage monitoring (optional)
Since the ESP32-C3 lacks native battery monitoring, use a voltage divider (two 200kΩ resistors) to halve the batter
y voltage and connect to A0:

float Vbattf = 2 * analogReadMilliVolts(A0) / 1000.0; // Battery voltage in volts


## software-defined

Sparkfun provides an Arduino library for controlling this device at: ????


## Notes

Links:
  * https://www.sparkfun.com/sparkfun-triad-spectroscopy-sensor-as7265x-qwiic.html
  * https://learn.sparkfun.com/tutorials/spectral-triad-as7265x-hookup-guide
  * https://github.com/sparkfun/Qwiic_Spectral_Sensor_AS7265X
  * https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library


