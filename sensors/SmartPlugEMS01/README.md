# Emporia EMS01 Smart Plug

**WIP**

## Hardware

* EMS01 Smart Plug Major Components
  - ICs
    * ESP8266
      - WiFi-equipped MCU
      - GPIO Pinout
        * GPIO4:  Relay
        * GPIO5:  BL0937 CF
        * GPIO12: BL0937 SEL
        * GPIO13: Push Button (High=off, Low=on)
        * GPIO14: BL0937 CF1
        * GPIO15: RED LED (High=on, Low=off)
      - ESP8266 board's edge connector (plug pins up, ground pin at bottom)
        * 1 (TOUT): ADC input, ?          ; * 12 (GND): Ground
        * 2 (EXT_RSTB): deep sleep wakeup ; * 11 (MTDO): GPIO15, Red LED
        * 3 (U0RXD): GPIO3, Rx            ; * 10 (U0TXD): GPIO1, Tx
        * 4 (GPIO2): TX'                  ; * 9 (MTCK): GPIO13, Button
        * 5 (?): ????                     ; * 8 (XPD_DCDC): ????
        * 6 (Vdd): 3V3                    ; * 7 (GPIO4): ????Relay????
    * BL0937
      - Single Phase Energy Meter IC
      - +/-0.5% error
      - current and voltage RMS
      - SOP8
      - 2x ADCs, DSP, Voltage to Frequency Converter
      - pinout
        * 1 (Vdd): 3.3V
        * 2 (IP), 3 (IN): differential current channel inputs, +/-50mV
        * 4 (VP): positive voltage channel input
        * 5 (GND): Ground
        * 6 (CF): high freq pulse output for active power, if over-current 6.78KHz
        * 7 (CF1): current/voltage RMS, 38usec (typ) pulse width
          - SEL=0: current RMS pulse
          - SEL=1: voltage RMS pulse
        * 8 (SEL): CF1 output selection
    * ZB25FQ16ATIG
      - 16Mb/2MB Serial NOR Flash
      - SPI bus
      - SOP8
      - Zbit Semi
      - Pins: CS#, DO, WP#, GND, DI, CLK, HOLD#/RESET#, VCC
      - 2.3-3.3V
  - LEDs
    * BLU: power on
    * RED: controlled by MCU (GPIO15)
  - Relay: switches power to socket (GPIO4)
  - Momentary Button: input (GPIO13)

* Flash Programming the ESP8266
  - use 3V3-capable USB-to-serial converter
  - GPIO0 must be low when booting to enter flash mode
  - Arduino IDE 2.3.6 can be used with USB-to-serial converter to flash
    * DTR: Reset
    * RTS: Boot Mode
  - Boot-mode operation
    * set strapping options: GPIO0: LOW, GPIO2: HIGH, GPIO15: LOW
    * Reset or power cycle the device
  - failure to flash
    * use serial monitor with baud=74880 and do a reset
    * if error says: boot mode(3,7)
      - trying to boot from flash (3), but bootloader is corrupt (7)
    * force to download mode
      - GPIO0=LOW, GPIO15=LOW, GPIO2=HIGH
    * reflash bootloader
      - erase the chip
        * 'esptool --port /dev/ttyUSB0 erase-flash'
      - flash complete factory image (bootloader + SDK + blank app)
        * 'esptool --port /dev/ttyUSB0 --flash_size 2MB --flash_mode qio --flash_freq 40m 0x0000 bootloader.bin'
    * 

* links
  - https://devices.esphome.io/devices/emporia_smart_plug/
  - ? 

**TODO move this to its own file**

* ESPHome Config Yaml
'''
# Basic Config
esphome:
  name: emporia_smart_plug

esp8266:
  board: esp01_1m

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

logger:

# Remove this line if you're not using Home Assistsant or your switch will restart every now and again
api:

ota:

# Device Specific Config
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO13
      mode: INPUT_PULLUP
      inverted: True
    name: "Emporia Smart Plug Button"
    on_press:
      - switch.toggle: relay
  - platform: status
    name: "Emporia Smart Plug Status"

sensor:
  - platform: wifi_signal
    name: "Emporia Smart Plug WiFi Signal"
    update_interval: 60s
  - platform: hlw8012
    model: BL0937
    sel_pin:
      number: 12
      inverted: true
    cf_pin: 5
    cf1_pin: 14
    current:
      name: "Emporia Smart Plug Current"
    voltage:
      name: "Emporia Smart Plug Voltage"
    power:
      name: "Emporia Smart Plug Power"
      id: my_power
    energy:
      name: "Emporia Smart Plug Energy"
    update_interval: 30s
  - platform: total_daily_energy
    name: "Emporia Smart Plug Daily Energy"
    power_id: my_power

switch:
  - platform: gpio
    name: "Emporia Smart Plug Relay"
    pin: GPIO4
    id: relay
    on_turn_on:
      - light.turn_on: status_light
    on_turn_off:
      - light.turn_off: status_light

time:
  - platform: sntp
    id: my_time

light:
  - platform: status_led
    pin: GPIO15
    id: status_light
    internal: true
'''