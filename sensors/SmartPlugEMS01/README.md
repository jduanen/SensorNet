# Emporia EMS01 Smart Plug Modified for Home Assistant Use

This sensor is a Home Assistant controllable power switch and load sensor that
is based on the Emporia EMS01 commercial product.
The EMS01 is reprogrammed to be an ESPHome-based Home Assistant device, that
doesn't require any subscriptions or cloud-based services.

## Hardware

### EMS01 Major Components

#### ESP8266EX

The ESP8266EX is a microcontroller with embedded WiFi in a 32-pin QFN package.
This MCU provides direct user interaction with a push button and LED, controls
the relay that provides power to the device's AC socket, gets current and
power information from the power meter chip, and communicates with Home
Assistant over WiFi.

The ESP8266 MCU is mounted on a board with its associated peripheral components
(e.g., resistors, capacitors, crystal, flash EEPROM, etc.). This module is
attached to the device's main board via a (soldered) edge connector.

##### ESP8266EX Pin Assignments

The diagram represents a top view of the component and the character `*` marks
the pins which are of interest here. Letters in parentheses associate pins to
the module's edge connector pins.

This is transcribed from the annotated printout in [SmartPlug.jpg](assets/SmartPlug.jpg), which
has handwritten notes on top of page 10 of the Espressif ESP8266EX datasheet.

```
                 (E)                        (F)                        TX       RX
                                                                       GPIO1    GPIO3
                 EXT_RSTB RES12K   VDDA     VDDD     XTAL_IN  XTAL_OUT U0TXD    U0RXD
                 32       31       30       29       28       27       *26      *25
                +------------------------------------------------------------------------+
        VDDA  1 | o                                                                      | *24 GPIO5  CF
         LNA  2 |                                                                        | 23  SD_DATA_1
      VDD3P3  3 |                                                                        | 22  SD_DATA_0
      VDD3P3  4 |                                                                        | 21  SD_CLK
     VDD_RTC  5 |                               ESP8266EX                                | 20  SD_CMD
(B)     TOUT  6 |                                                                        | 19  SD_DATA_3
     CHIP_EN  7 |                                                                        | 18  SD_DATA_2
(A) XPD_DCDC  8 |                                                              33 GND    | 17  VDDPST
                +------------------------------------------------------------------------+
                 *9       *10      11       *12      *13      14       *15      *16
                 MTMS     MTDI     VDDPST   MTCK     MTDO     GPIO2    GPIO0    GPIO4
                 GPIO14   GPIO12            GPIO13   GPIO15   TX'
                 CF1      SEL               BTN      RED LED  (D)      (boot)   RELAY (C)
```

| Pin | Signal Name | GPIO   | Edge | Function          |
|----:|-------------|--------|------|-------------------|
|   6 | TOUT        |        | (B)  | ADC               |
|   8 | XPD_DCDC    |        | (A)  | Deep sleep wakeup |
|   9 | MTMS        | GPIO14 |      | BL0937 CF1        |
|  10 | MTDI        | GPIO12 |      | BL0937 SEL        |
|  12 | MTCK        | GPIO13 |      | Push button       |
|  13 | MTDO        | GPIO15 |      | Red LED           |
|  14 | GPIO2       | GPIO2  | (D)  | TX'               |
|  15 | GPIO0       | GPIO0  |      | Boot mode         |
|  16 | GPIO4       | GPIO4  | (C)  | Relay control     |
|  24 | GPIO5       | GPIO5  |      | BL0937 CF         |
|  25 | U0RXD       | GPIO3  |      | UART RX           |
|  26 | U0TXD       | GPIO1  |      | UART TX           |
|  29 | VDDD        |        | (F)  | Vdd 3V3           |
|  32 | EXT_RSTB    |        | (E)  | External reset    |

##### GPIO Pin Usage

Several of the MCU's GPIO pins are used to sense and control the state of the
smart plug. The GPIO pin and its function are given in the table below.

|-- Pin --|-- Function --|
| GPIO4 | Relay |
| GPIO5 | BL0937 CF |
| GPIO12 | BL0937 SEL |
| GPIO13 | Push Button (High=off, Low=on) |
| GPIO14 | BL0937 CF1 |
| GPIO15 | RED LED (High=on, Low=off) |

  - GPIO4:  Relay
  - GPIO5:  BL0937 CF
  - GPIO12: BL0937 SEL
  - GPIO13: Push Button (High=off, Low=on)
  - GPIO14: BL0937 CF1
  - GPIO15: RED LED (High=on, Low=off)

##### ESP8266 Module Edge Connector Pinout

(as seen when looking at the the device from the back side of the main board,
with the plug's ground pin at bottom and the module connections to the right
of the plug)

|:-- Left Pin # (Name) --|-- Pin Function --|:-- Right Pin # (Name) --|-- Pin Function --|
| 1 (TOUT) | ADC input | 12 (GND) | Ground |
| 2 (EXT_RSTB) | External Reset | 11 (MTDO) | GPIO15, Red LED |
| 3 (U0RXD) | GPIO3, Rx | 10 (U0TXD) | GPIO1, Tx |
| 4 (GPIO2) | TX' | 9 (MTCK) | GPIO13, Button |
| 5 (GPIO4) | ?Relay? | 8 (XPD_DCDC) | Deep Sleep Wakeup |
| 6 (Vdd) | 3V3 | 7 (GPIO4) | ?Relay? |

  - 1 (TOUT):     ADC input       ; - 12 (GND):      Ground
  - 2 (EXT_RSTB): External Reset  ; - 11 (MTDO):     GPIO15, Red LED
  - 3 (U0RXD):    GPIO3, Rx       ; - 10 (U0TXD):    GPIO1, Tx
  - 4 (GPIO2):    TX'             ; -  9 (MTCK):     GPIO13, Button
  - 5 (GPIO4):    ?Relay?         ; -  8 (XPD_DCDC): Deep Sleep Wakeup
  - 6 (Vdd):      3V3             ; -  7 (GPIO4):    ?Relay?

Twelve pads in two columns either side of the board, drawn with the plug's pins
up. The programming connections are given in square brackets, matching the
[wiring table](#programming-wiring).

```
                      _
                  ___|_|___
                 |___| |___|
                     | |
          ADC --> [1]| |[12] <-- GND     [BLK]
[BLU] EXT_RST --> [2]| |[11] --> RED LED
[GRN]     RX  --> [3]| |[10] --> TX      [YEL]
[ORG]   GPIO2 --> [4]| |[9]  <-- BTN
        RELAY <-- [5]| |[8]  <-- XPD_DCDC
[RED] Vdd 3V3 --> [6]|_|[7]  <-- BOOT    [WHT]

```

#### BL0937 Energy Meter Chip

????

```
              +----\_/----+
(VDDD) VDD ---| 1       8 |--- SEL  (GPIO12)
        IP ---| 2       7 |--- CF1  (GPIO14)
        IN ---| 3       6 |--- CF   (GPIO5)
        VP ---| 4       5 |--- GND
              +-----------+
                  BL0937
```

????

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

#### Flash Chip

????

* ZB25FQ16ATIG
  - 16Mb/2MB Serial NOR Flash
  - SPI bus
  - SOP8
  - Zbit Semi
  - Pins: CS#, DO, WP#, GND, DI, CLK, HOLD#/RESET#, VCC
  - 2.3-3.3V

#### Button and LEDs

- LEDs
  * BLU: power on
  * RED: controlled by MCU (GPIO15)

- Momentary Button: input (GPIO13)

```
   GPIO13                  GPIO15
     ^                       |
     |                       v
     +---o                  _V_  RED
         |=|                 |
     +---o                  [R]
     |                       |
    GND                     GND
```

#### Power Relay

????
switches power to socket (GPIO4)

## Reprogramming the EMS01

????

### ESP8266 Boot-Mode Strapping

The ESP8266 must be set into download mode. This requires that GPIO15 and GPIO0
are held low and GPIO2 high while the part is reset.

????

```
                     Boot Mode
                      [ ]| |[ ]
                      [ ]| |[■]--+   GPIO15
                      [ ]| |[ ]  |
                                GND
       3V3
        o
        |
        +------------ [■]| |[ ]        GPIO2
                      [ ]| |[ ]
                      [ ]|_|[■]--+   GPIO0
                                 |
                                GND
```

### Using Jigs to Enable Programming

In order to get access to the required signals, a series of holes must be
drilled into the back of the device's case.

A jig is used to drill 1mm holes into the plastic case, which provides access
to the ESP8266 module's edge connector pins.

#### Hole Pattern Drilling Jig

??image here??

In the diagram below, the filled holes are populated, and the layout
matches that of the edge connector's solder pads on the device's main
PCB.

```
                     o   ●  BLK: Ground
       Reset: BLU    ●   o
          RX: GRN    ●   ●  YEL: TX
       GPIO2: ORG    ●   o
                     o   o
         3V3: RED    ●   ●  WHT: Boot
```

#### Programming Jig Attachment
Once the holes are drilled, a jig with pogo-pins is placed onto the back of the
EMS01 and a (3V3) USB UART device is connected to the wires coming from the pogo
pins. This provides a USB connection to a (Linux/Windows) machine that runs the
software that reprograms the device.

Following the reprogramming, the jig is removed and the holes are sealed. All
subsequent programming of the device can be done over the WiFi connection
(i.e., OTA programming).

??image here??

????

#### USB Adapter Wiring

The final wiring provides the connection from the programming jig to a (3V3)
USB-to-serial converter.

```
 USB-serial    Plug edge connector
 ----------    -------------------
  DTR  ---- BLU ---> External Reset (EXT_RSTB)
  TX   ---- GRN ---> RX (U0RXD/GPIO3)
  3V3  ---- RED ---> Vdd (VDDD)
  RTS  ---- WHT ---> Boot mode (GPIO0)
  RX   <--- YEL ---- TX (U0TXD/GPIO1)
  GND  ---- BLK ---- GND
  ???  ---- ORG ---- ? (TX'/GPIO2)
```

"Device" names the signal at the plug. "Converter" names the signal at
the USB-serial end, so RX and TX cross over.

| # | Device    | Color | Converter | Color |
|--:|-----------|-------|-----------|-------|
| 1 | Reset     | GRN   | Reset/DTR | BLU   |
| 2 | Rx        | ORG   | Tx        | GRN   |
| 3 | 3V3       | RED   | 3V3       | RED   |
| 4 | Boot mode | YEL   | Boot/RTS  | WHT   |
| 5 | Tx        | BRN   | Rx        | YEL   |
| 6 | GND       | BLK   | GND       | BLK   |


## Open questions

1. **Flash part number.** The sheet reads `25VG16ATIG`, and the "G" may be a
   "Q". The README says `ZB25FQ16ATIG`. Check against the chip.
2. **Bottom-right edge pad.** The sketch shows GPIO0 / Boot mode, and the jig
   and wiring tables agree. The README's edge connector list has
   `7 (GPIO4): ????Relay????` there.
3. **Relay pad letter.** The circled letter beside RELAY on the edge connector
   and at chip pin 16 is blacked out. It is read as C from the legend.
4. **Pad markings.** The edge connector pads are drawn with several fills
   (solid, red, crossed, grid, empty). The sheet doesn't define them, so the
   diagram above omits them, apart from noting the red pads.
5. **GPIO2 in the jig.** The jig pattern has GPIO2 populated with an ORG wire,
   but no final wiring table includes it. The README says to leave GPIO2
   unconnected and rely on its pull-up.
6. **`JG2436`.** Written above the flash part number, with no explanation. It
   may be a lot or date code.


## Notes

* Flash Programming the ESP8266
  - use 3V3-capable USB-to-serial converter
  - GPIO0 must be low when booting to enter flash mode
  - GPIO2 must be left unconnected (internally pulled-up)
  - On power-up, UART0Tx outputs chars
    * 115200 baud (@40MHz) or 74880 baud (@27MHz)
  - Arduino IDE 2.3.6 can be used with USB-to-serial converter to flash
    * DTR: Reset
    * RTS: Boot Mode
  - Boot-mode operation
    * set strapping options: GPIO0: LOW, GPIO2: HIGH, GPIO15: LOW
    * Reset or power cycle the device
  - failure to flash
    * use serial monitor with baud=74880 and do a reset
    * error codes
      - "boot mode:(3,7)"
        * trying to boot from flash (3), but bootloader is corrupt (7)
      - "boot mode:(1, 6)"
        * booting from UART (1), strapping pin states (GPIO0=LOW, GPIO2=HIGH, GPIO15=LOW)
        * strapping is correct, device is in the bootloader (download mode)
      - "boot mode:(3,6)"
        * normal boot
    * force to download mode
      - GPIO0=LOW, GPIO15=LOW, GPIO2=HIGH
    * reflash bootloader
      - erase the chip
        * `esptool --port /dev/ttyUSB0 erase-flash`  #### bad
        * `esptool --chip esp8266 --baud 74880 --port /dev/ttyUSB0 --before usb-reset --connect-attempts 0 erase-flash`
      - flash complete factory image (bootloader + SDK + blank app)
        * `esptool --port /dev/ttyUSB0 --flash_size 2MB --flash_mode qio --flash_freq 40m 0x0000 bootloader.bin`  #### bad
  - Programming Jig to FTDI Converter board connections
    * pin: board signal - wire color (device function)
      - 1: DTR - BLU (Reset)
      - 2: TX - GRN (Rx)
      - 3: 3V3 - RED (Vdd)
      - 4: RTS - WHT (Boot Mode)
      - 5: RX - YEL (Tx)
      - 6: GND - BLK (Ground)
    * boot mode
      - WHT = GPIO0 = LOW
      - N.C. = GPIO2 = HIGH
      - N.C. = GPIO15 = LOW
  - Procedure to force the UART bootloader (download mode)
    * disconnect the plug from mains before connecting the converter
      - the plug's power supply may not be isolated
    * connect the jig, using 3V3 logic only
      - power must be a stable 3V3 source capable of 300-500mA
    * hold GPIO0 (WHT) to GND
    * power cycle, or pulse EXT_RSTB (BLU) low
    * keep GPIO0 low for 1-2 secs after power-up, then it can be released
    * confirm the device is in the bootloader
      - `esptool --port /dev/ttyUSB0 flash-id`
      - or look for "boot mode:(1,x)" with serial monitor at 74880 baud
    * flash the device
      - `esphome upload --device /dev/ttyUSB0 smart-plug-ems01-0.yaml`
    * release GPIO0 and power cycle to run the new firmware
      - the device stays in the bootloader until it is reset
  
### Links
  - https://devices.esphome.io/devices/emporia_smart_plug/
  - ? 
