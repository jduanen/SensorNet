# HA_Remote
A remote display and input device for Home Assistant

## Hardware

It is important to know that LilyGo silently switched the T-Encoder Pro to a second hardware revision (TFD12MASBCTB4_V0_07) that uses a CO5300 display and CST816 touch. Their own pin_config.h even comments out the SH8601A variant and has #define TFD12MASBCTB4_V0_07 as the default. The CHSC5816 I2C address is 0x2E, and the CST816 is at 0x15 (which can be used to confirm the presence of a new variant).

Furthermore, the CO5300 powers on in single-SPI mode and requires [0xC4, 0x80] to activate QSPI. Every command before that works on SDIO0 alone, and only after that does it start listening on all 4 data lines.
The LilyGo Arduino driver sends this command early in init; neither of the previous configs included it.

* Lily Go T-Encoder-Pro
  - ESP32-S3 R8, WiFi and BTLE5
  - Flash: 16 MB
  - PSRAM: 8 MB
  - 2x Qwiic 4x pin
    * GND, 3V3, SDA (GPIO16), SCL (GPIO15)
    * GND, 5V, TX (GPIO43), RX (GPIO44)
  - USB-C
  - Display CO5300
    * 1.2" 390x390 AMOLED
    * QSPI interface
      - LCD_CS:     GPIO10
      - LCD_RST:    GPIO04
      - LCD_SDIO0:  GPIO11
      - LCD_SDIO1:  GPIO13
      - LCD_SDIO2:  GPIO07
      - LCD_SDIO3:  GPIO14
      - LCD_SCLK:   GPIO12
      - LCD_VCI_EN: GPIO03
  - CST816
    * Touchscreen Controller
      - SDA: GPIO05
      - SCL: GPIO06
      - RST: GPIO08
      - INT: GPIO09
    * Rotary Encoder
      - KNOB_DATA_A: GPIO01
      - KNOB_DATA_B: GPIO02
      - KNOB_KEY:    GPIO00
  - Buzzer
    * BUZZER_DATA: GPIO17
  - 5V @ 500mA
  - 43.5 x 27.5mm

* Wireless Battery Charging
  - ???? Qi Wireless Charging Receiver
    * ?
  - Sparkfun Battery Babysitter
    * Allows charging of battery and suppling the output load with 5V
    * Charging input: 4.35-6.4V
    * Load output voltage range: 3.0-5.5V
      - if battery is charging, output voltage will be around 4.3V, but may drop as low as 3.8V depending on the load
      - if only a battery is present, the output should be about the battery voltage (minus a small drop)
      - if battery and 5V USB are connected, and the battery is charging, the output will be 3.8 - 4.3V
      - if battery and 5V USB are connected, and the battery is fully charged, the output will be 4.5 - 5V.
    * ON/OFF switch toggles the "SYSOFF" pin of the BQ24075
      - allows you to disconnect the battery from the system output
      - effectively controlling power to a load when the Babysitter is in battery-powered operation
      - to enable charging, the switch must be in the ON position
        * battery charging will be disabled if the switch is off
      - if a charge supply is connected the system output voltage will be enabled
        * regardless of the state of the switch
    * LEDs
      - BLU: on = charging, off = charging complete
      - RED: on = output power available, off = no output power
    * DIP Switches (EN2/EN1)
      - 0/0: Suspend Mode (no charging)
      - 0/1: Fast Charge (1.5A with ISET)
      - 1/0: 500mA charge
      - 1/1: 100mA charge (default) -- using this
  - Battery Level Meter
    * ????

## Notes

* Links
  - HA Forum
    * https://community.home-assistant.io/t/lilygo-t-encoder-pro-config-for-esp-home/811727
  - GitHub
    * https://github.com/Xinyuan-LilyGO/T-Encoder-Pro
  - CHSC5816 ESPHome Driver
    * https://github.com/mvonweis/esphome-chsc5816/
  - ?
    * ?
