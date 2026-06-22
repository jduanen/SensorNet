# HA_Remote
A remote display and input device for Home Assistant

## Hardware

* Lily Go T-Encoder-Pro
  - ????

* Wireless Battery Charging
  - ???? Qi Wireless Charging Client
    * ?
  - Sparkfun Battery Babysitter
    * Allows charging of battery and suppling the output load with 5V
    * Charge input: 4.35-6.4V
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
    * Output Voltage Range: 3.0-5.5V
      - if battery is charging, output voltage will be around 4.3V, but may drop as low as 3.8V depending on the load
      - if only a battery is present, the output should be about the battery voltage (minus a small drop)
      - if battery and 5V USB are connected, and the battery is charging, the output will be 3.8 - 4.3V
      - if battery and 5V USB are connected, and the battery is fully charged, the output will be 4.5 - 5V.
  - Battery Level Meter
    * ????

## Notes

### Links

* https://community.home-assistant.io/t/lilygo-t-encoder-pro-config-for-esp-home/811727
* https://github.com/Xinyuan-LilyGO/T-Encoder-Pro

