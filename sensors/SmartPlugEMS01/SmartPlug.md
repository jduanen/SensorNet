# EMS01 Smart Plug: Hardware Notes

Transcribed from the annotated printout in [SmartPlug.jpg](SmartPlug.jpg), which
is page 10 of the ESP8266EX datasheet with handwritten notes around it.
Items that could not be read with certainty are listed under
[Open questions](#open-questions).

## ESP8266EX pin assignments

32-pin QFN, top view. `*` marks the pins highlighted in yellow on the printout.
Pin 15 (GPIO0) also has a red arrow. Letters refer to the
[legend](#legend).

```
                 E                                                     TX       RX
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

| Pin | Datasheet name | GPIO   | Function in the plug         |
|----:|----------------|--------|------------------------------|
|   6 | TOUT           |        | (B) ADC                      |
|   8 | XPD_DCDC       |        | (A) Deep sleep wakeup        |
|   9 | MTMS           | GPIO14 | BL0937 CF1                   |
|  10 | MTDI           | GPIO12 | BL0937 SEL                   |
|  12 | MTCK           | GPIO13 | Push button                  |
|  13 | MTDO           | GPIO15 | Red LED                      |
|  14 | GPIO2          | GPIO2  | (D) TX'                      |
|  15 | GPIO0          | GPIO0  | Boot mode                    |
|  16 | GPIO4          | GPIO4  | (C) Relay                    |
|  24 | GPIO5          | GPIO5  | BL0937 CF                    |
|  25 | U0RXD          | GPIO3  | UART RX                      |
|  26 | U0TXD          | GPIO1  | UART TX                      |
|  32 | EXT_RSTB       |        | (E) External reset           |

### Legend

| Letter | Meaning           |
|--------|-------------------|
| A      | Deep sleep wakeup |
| B      | ADC               |
| C      | Relay             |
| D      | GPIO2, TX'        |
| E      | External reset    |

## Board edge connector

Twelve pads in two columns either side of the board, drawn with the plug's pins
up. Boxed numbers are the programming connections, matching the
[wiring table](#programming-wiring).

```
                 (plug pins)
                  ___|_|___
                 |___| |___|
                     | |
             (B)  [ ]| |[ ]  GND          <-- [6]
 [1] --> (E)      [ ]| |[ ]  RED LED
 [2] --> RX       [ ]| |[ ]  TX           <-- [5]
             (D)  [ ]| |[ ]  BTN
 RELAY (C)        [ ]| |[ ]  (A)
 [3] --> Vdd 3V3  [ ]|_|[ ]  GPIO0 - Boot mode  <-- [4]
```

The Relay and GPIO0 pads are coloured red on the sketch.

## Programming jig hole pattern

Printed along the top of the page. Filled holes are populated, and the layout
is the same as the edge connector.

```
                     o   ●  BLK: Ground
       Reset: BLU    ●   o
          RX: GRN    ●   ●  YEL: TX
       GPIO2: ORG    ●   o
                     o   o
         3V3: RED    ●   ●  WHT: Boot
```

## Programming wiring

The final wiring is the set in green ink, from a USB-to-serial converter to the
jig. The circled numbers match the boxed numbers on the edge connector.

```
  USB-serial                        Plug edge connector
  ----------                        -------------------
  DTR  ------------- BLU ---- [1] ---> E   External reset (EXT_RSTB)
  TX   ------------- GRN ---- [2] ---> RX  U0RXD / GPIO3
  3V3  ------------- RED ---- [3] ---> Vdd
  RTS  ------------- WHT ---- [4] ---> GPIO0  Boot mode
  RX   <------------ YEL ---- [5] ---- TX  U0TXD / GPIO1
  GND  ------------- BLK ---- [6] ---- GND
```

### Wire colours: earlier and final

The sheet records an earlier black-ink colour set, with the green set written
over it. "Device" names the signal at the plug. "Converter" names the signal at
the USB-serial end, so RX and TX cross over.

| # | Device     | Earlier colour | Converter | Earlier colour | Final colour |
|--:|------------|----------------|-----------|----------------|--------------|
| 1 | Reset      | GRN            | Reset/DTR | GRN            | BLU          |
| 2 | Rx         | ORG            | Tx        | ORG            | GRN          |
| 3 | 3V3        | RED            | 3V3       | RED            | RED          |
| 4 | Boot mode  | YEL            | Boot/RTS  | YEL            | WHT          |
| 5 | Tx         | BRN            | Rx        | BRN            | YEL          |
| 6 | GND        | BLK            | GND       | BLK            | BLK          |

## Boot mode strapping

Download mode needs GPIO15 and GPIO0 low and GPIO2 high.

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

## BL0937 energy meter

```
              +----\_/----+
       VDD ---| 1       8 |--- SEL  (GPIO12)
        IP ---| 2       7 |--- CF1  (GPIO14)
        IN ---| 3       6 |--- CF   (GPIO5)
        VP ---| 4       5 |--- GND
              +-----------+
                  BL0937
```

The sketch does not number these pins. The numbers above come from the BL0937
pinout in the [README](../README.md).

## Button and LED

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

## Flash chip

Handwritten markings: `JG2436`, `25VG16ATIG`, "16Mb NOR serial".

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
