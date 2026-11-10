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
(e.g., resistors, capacitors, crystal, flash memory, etc.). This module is
attached to the device's main board via a (soldered) edge connector.

##### ESP8266EX Pin Assignments

The diagram represents a top view of the component and the character `*` marks
the pins which are of interest here. Letters in parentheses are the labels used
on the annotated printout. The `Edge` column in the table gives the
corresponding [edge connector](#esp8266-module-edge-connector-pinout) pin.

This is transcribed from the annotated printout in
[SmartPlug.jpg](assets/SmartPlug.jpg), which has handwritten notes on top of
page 10 of the [Espressif ESP8266EX datasheet](docs/0a-esp8266ex_datasheet_en.pdf).

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

| Pin | Signal Name | GPIO   | Label | Edge | Function                            |
|----:|-------------|--------|-------|-----:|-------------------------------------|
|   6 | TOUT        |        | (B)   |    1 | ADC                                 |
|   8 | XPD_DCDC    |        | (A)   |    8 | Deep sleep wakeup                   |
|   9 | MTMS        | GPIO14 |       |      | BL0937 CF1                          |
|  10 | MTDI        | GPIO12 |       |      | BL0937 SEL                          |
|  12 | MTCK        | GPIO13 |       |    9 | Push button (pressed = low)         |
|  13 | MTDO        | GPIO15 |       |   11 | Red LED (high = on); boot strap     |
|  14 | GPIO2       | GPIO2  | (D)   |    4 | TX'; boot strap                     |
|  15 | GPIO0       | GPIO0  |       |    7 | Boot mode strap                     |
|  16 | GPIO4       | GPIO4  | (C)   |    5 | Relay control                       |
|  24 | GPIO5       | GPIO5  |       |      | BL0937 CF                           |
|  25 | U0RXD       | GPIO3  |       |    3 | UART RX                             |
|  26 | U0TXD       | GPIO1  |       |   10 | UART TX                             |
|  29 | VDDD        |        | (F)   |    6 | Vdd 3V3                             |
|  32 | EXT_RSTB    |        | (E)   |    2 | External reset                      |

##### ESP8266 Module Edge Connector Pinout

(as seen when looking at the device from the back side of the main board,
with the plug's ground pin at bottom and the module connections to the right
of the plug)

<table>
  <thead>
    <tr>
      <th colspan="3">Left Pins Column</th>
      <th colspan="3">Right Pins Column</th>
    </tr>
    <tr>
      <th>Function</th>
      <th>Name</th>
      <th>Pin #</th>
      <th>Pin #</th>
      <th>Name</th>
      <th>Function</th>
    </tr>
  </thead>
  <tbody>
    <tr><td>ADC input</td><td>TOUT</td><td>1</td><td>12</td><td>GND</td><td>Ground</td></tr>
    <tr><td>External Reset</td><td>EXT_RSTB</td><td>2</td><td>11</td><td>MTDO</td><td>GPIO15, Red LED</td></tr>
    <tr><td>GPIO3, Rx</td><td>U0RXD</td><td>3</td><td>10</td><td>U0TXD</td><td>GPIO1, Tx</td></tr>
    <tr><td>TX'</td><td>GPIO2</td><td>4</td><td>9</td><td>MTCK</td><td>GPIO13, Button</td></tr>
    <tr><td>Relay</td><td>GPIO4</td><td>5</td><td>8</td><td>XPD_DCDC</td><td>Deep Sleep Wakeup</td></tr>
    <tr><td>3V3</td><td>VDDD</td><td>6</td><td>7</td><td>GPIO0</td><td>Boot Mode</td></tr>
  </tbody>
</table>

The same twelve pads, drawn with the plug's pins up. The programming wire
colors are given in parentheses, matching the
[USB adapter wiring](#usb-adapter-wiring).

```
                      _
                  ___|_|___
                 |___| |___|
                     | |
          ADC --> [1]| |[12] <-- GND     (BLK)
(BLU) EXT_RST --> [2]| |[11] --> RED LED
(GRN)     RX  --> [3]| |[10] --> TX      (YEL)
(ORG)   GPIO2 --> [4]| |[9]  <-- BTN
        RELAY <-- [5]| |[8]  <-- XPD_DCDC
(RED) Vdd 3V3 --> [6]|_|[7]  <-- BOOT    (WHT)

```

#### BL0937 Energy Meter Chip

The BL0937 ([datasheet](docs/BL0937_V1.02_en.pdf)) is a single-phase energy
meter IC in an SOP8 package. It has two ADCs, a DSP, and voltage-to-frequency
converters. It measures current and voltage RMS and active power with +/-0.5%
error, and reports them as pulse trains that the ESP8266 counts.

```
              +----\_/----+
(VDDD) VDD ---| 1       8 |--- SEL  (GPIO12)
        IP ---| 2       7 |--- CF1  (GPIO14)
        IN ---| 3       6 |--- CF   (GPIO5)
        VP ---| 4       5 |--- GND
              +-----------+
                  BL0937
```

| Pin | Name | Description |
|----:|------|-------------|
| 1 | VDD | 3.3V supply |
| 2 | IP  | Current channel differential input (+), +/-50mV |
| 3 | IN  | Current channel differential input (-) |
| 4 | VP  | Voltage channel positive input |
| 5 | GND | Ground |
| 6 | CF  | Active power pulse output (high frequency); 6.78KHz on over-current |
| 7 | CF1 | Current or voltage RMS pulse output, 38usec (typ) pulse width |
| 8 | SEL | CF1 output select: 0 = current RMS, 1 = voltage RMS |

#### Flash Chip

The ESP8266 module's firmware is stored in a 16Mb (2MB) serial NOR flash
chip from Zbit Semi, in an SOP8 package, on the SPI bus.

* Part number: `ZB25FQ16ATIG` (see [Open Questions](#open-questions))
* Pins: CS#, DO, WP#, GND, DI, CLK, HOLD#/RESET#, VCC
* Supply: 2.3-3.3V

#### Button and LEDs

- LEDs
  * BLU: power on
  * RED: controlled by MCU (GPIO15), high = on
- Momentary Button: input (GPIO13), low when pressed

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
The relay switches power to the socket, and is controlled by GPIO4.

## Reprogramming the EMS01

The factory firmware is replaced with ESPHome by flashing the ESP8266 over its
serial port, using a 3V3 USB-to-serial converter. This requires access to the
module's edge connector through holes drilled in the case. After the first
flash, all later updates are done over WiFi (OTA).

**Warning:** disconnect the plug from mains before connecting the converter.
The plug's power supply may not be isolated.

### ESP8266 Boot-Mode Strapping

The ESP8266 must be set into download mode. This requires that GPIO15 and GPIO0
are held low and GPIO2 high while the part is reset. On the EMS01, GPIO15 is
pulled low on the board and GPIO2 is pulled up internally, so only GPIO0
has to be driven (low) by the programming jig.

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

| Strap  | Level | Provided by |
|--------|-------|-------------|
| GPIO0  | LOW   | WHT wire (RTS) |
| GPIO2  | HIGH  | Not connected (internal pull-up) |
| GPIO15 | LOW   | Not connected (pulled low on board) |

### Using Jigs to Enable Programming

In order to get access to the required signals, a series of holes must be
drilled into the back of the device's case.

A jig is used to drill 1mm holes into the plastic case, which provides access
to the ESP8266 module's edge connector pins.

#### Hole Pattern Drilling Jig

????
[Drilling Jig 1](assets/drillingJig1.gif)
[Drilling Jig 2](assets/drillingJig2.gif)
[Drilling Jig 3](assets/drillingJig3.gif)
????

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

????
[EMS01 holes](assets/EMS01_holes.gif)
????

#### Programming Jig Attachment

Once the holes are drilled, a jig with pogo-pins is placed onto the back of the
EMS01 and a (3V3) USB UART device is connected to the wires coming from the pogo
pins. This provides a USB connection to a (Linux/Windows) machine that runs the
software that reprograms the device.

Following the reprogramming, the jig is removed and the holes are sealed. All
subsequent programming of the device can be done over the WiFi connection
(i.e., OTA programming).

????
[Programming Jig](assets/programmingJig.gif)
????

#### USB Adapter Wiring

The final wiring provides the connection from the programming jig to a (3V3)
USB-to-serial converter. RX and TX cross over between the converter and the
device.

```
 USB-serial    Plug edge connector
 ----------    -------------------
  DTR  ---- BLU ---> [2]  External Reset (EXT_RSTB)
  TX   ---- GRN ---> [3]  RX (U0RXD/GPIO3)
  3V3  ---- RED ---> [6]  Vdd (VDDD)
  RTS  ---- WHT ---> [7]  Boot mode (GPIO0)
  RX   <--- YEL ---- [10] TX (U0TXD/GPIO1)
  GND  ---- BLK ---- [12] GND
  ???  ---- ORG ---- [4]  ? (TX'/GPIO2)
```

| # | Converter | Color | Device    | Edge |
|--:|-----------|-------|-----------|-----:|
| 1 | DTR       | BLU   | Reset     |    2 |
| 2 | TX        | GRN   | Rx        |    3 |
| 3 | 3V3       | RED   | 3V3       |    6 |
| 4 | RTS       | WHT   | Boot mode |    7 |
| 5 | RX        | YEL   | Tx        |   10 |
| 6 | GND       | BLK   | GND       |   12 |

????
[USB Adaptor](assets/USB_adaptor.gif)
????

Notes:
* The 3V3 supply must be stable and able to source 300-500mA. Use 3V3 logic
  levels only.
* The same DTR/RTS wiring lets tools like Arduino IDE (2.3.6) and esptool
  drive reset and boot mode automatically.
* An earlier version of this cable used different colors (Reset GRN, Rx ORG,
  Boot YEL, Tx BRN). The printout shows them in black ink, with the current
colors written over them in green.

### Flashing Procedure

1. Disconnect the plug from mains.
2. Attach the programming jig and connect the USB-to-serial converter.
3. Hold GPIO0 (WHT) to GND.
4. Power cycle, or pulse EXT_RSTB (BLU) low.
5. Keep GPIO0 low for 1-2 secs after power-up, then it can be released.
6. Confirm the device is in the bootloader:
   * `esptool --port /dev/ttyUSB0 flash-id`
   * or look for `boot mode:(1,x)` with a serial monitor at 74880 baud
7. Flash the device:
   * `esphome upload --device /dev/ttyUSB0 smart-plug-ems01-0.yaml`
8. Release GPIO0 and power cycle to run the new firmware. The device stays in
   the bootloader until it is reset.

### Troubleshooting

On power-up the ROM bootloader prints a status line on UART0 TX at 74880 baud
(26MHz crystal) or 115200 baud (40MHz crystal). Open a serial monitor at
that rate and reset the device.

| Message | Meaning |
|---------|---------|
| `boot mode:(3,6)` | Normal boot from flash |
| `boot mode:(3,7)` | Trying to boot from flash (3), but the bootloader is corrupt (7) |
| `boot mode:(1,6)` | Booting from UART (1): strapping is correct (GPIO0=LOW, GPIO2=HIGH, GPIO15=LOW) and the device is in the bootloader (download mode) |

To reflash the bootloader:
* Erase the chip:
  * `esptool --chip esp8266 --baud 74880 --port /dev/ttyUSB0 --before usb-reset --connect-attempts 0 erase-flash`
  * (`esptool --port /dev/ttyUSB0 erase-flash` did not work)
* Flash a complete factory image (bootloader + SDK + blank app):
  * `esptool --port /dev/ttyUSB0 --flash_size 2MB --flash_mode qio --flash_freq 40m 0x0000 bootloader.bin` did not work.
  * ????

## Open Questions

1. **Flash part number.** The printout reads `25VG16ATIG`, and the "G" may be a
   "Q" (i.e., Zbit `ZB25VQ16A`). This README previously said `ZB25FQ16ATIG`.
   Check against the chip.
2. **Relay pad letter.** The circled letter beside RELAY on the edge connector
   and at chip pin 16 is blacked out on the printout. It is read as (C) from
   the legend.
3. **Pad markings.** The edge connector pads on the printout are drawn with
   several fills (solid, red, crossed, grid, empty). Their meaning isn't
   given, so they are omitted here.
4. **GPIO2 in the jig.** The jig pattern has GPIO2 populated with an ORG wire,
   but the final wiring doesn't use it, and GPIO2 should be left unconnected
   so its pull-up holds it high at boot.
5. **`JG2436`.** Written above the flash part number on the printout, with no
   explanation. It may be a lot or date code.

## Links

- https://devices.esphome.io/devices/emporia_smart_plug/
- ?
