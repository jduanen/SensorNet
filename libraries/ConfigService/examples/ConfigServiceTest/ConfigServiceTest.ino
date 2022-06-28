/***************************************************************************
 *
 * Configuration Service Test
 *
 ***************************************************************************/

#include <Arduino.h>
#include "ConfigService.h"


#define VERBOSE         1
#define APP_NAME        "ConfigServiceTest"
#define APP_VERSION     "1.0.0"


void print(String s) {
    if (VERBOSE) {
      sn.consolePrint(s);
    }
}

void println(String s) {
    if (VERBOSE) {
      sn.consolePrintln(s);
    }
}

void setup() {
    sn.serialStart(&Serial, 9600, true);
    delay(500);
   println(APP_NAME);
}

void loop() {
};
