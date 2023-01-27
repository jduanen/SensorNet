//----------------------------------------------------------------------------------------------+
//               PIN MAP for  ATmega328P - Each I/O pin used is defined . . .
//----------------------------------------------------------------------------------------------+


// PIN MAP - Each I/O pin (used or unused) is defined . . .
#define PULSE_PIN         19             // digital one pulse pin modification
//                        18             // (A4) RESERVED for I2C
#define IR_PIN            17             // (A3)Interrupt 1 for IR sensor
#define SPKR_MUTE         16             // (A2) signal pin to mute speaker
#define NULL_BUTTON       15             // (A1) Null button used to set the null point in tone mode
#define TONE_POT          14             // (A0) tone adjustment via pot
#define LED_PIN           13             // for debug only - flashes 5X at startup
#define TONE_PIN          12             // PWM output to speaker or piezo for tone mode
#define SEL_BUTTON        11             // button to toggle alternate display and set alarm
#define ALARM_PIN         10             // Outputs HIGH when Alarm triggered
#define TUBE_SEL           9             // jumper to select alt conversion to uSv
#define LCDPIN_D7          8             // LCD D7 pin
#define LCDPIN_D6          7             // LCD D6 pin
#define LCDPIN_D5          6             // LCD D5 pin
#define LCDPIN_D4          5             // LCD D4 pin
#define LCDPIN_EN          4             // LCD enable pin
#define LCDPIN_RS          3             // LCD register select (RS) pin
//                         D2               Interrupt 0 for Geiger 
//                         D1 & D0          serial comm

// defines for LCD if DogM display used
#define DOGM_RST          -1             // Reset not used - PIN 8 is FREE
#define DOGM_CSB           7             // chip select 
#define DOGM_BKLT          6             // backlight on PWM pin - DOGM lib can dim
#define DOGM_CLK           5             // clock
#define DOGM_SI            4             // serial in
#define DOGM_RS            3             // register select
#define DOGM_CONTRAST     40             // value between 0-63 - 40 is perfect
#define DOGM_BKL_LOW      64             // backlight to low brightness (0-255 - 0=off)
#define DOGM_BKL_MED     128             // backlight to medium brightness
#define DOGM_BKL_HIGH    250             // backlight to high brightness
#define DOGM_BKL_OFF       0             // backlight off

//----------------------------------------------------------------------------------------------+
//                                 other defines . . .
//----------------------------------------------------------------------------------------------+

#define LOW_VCC            4200 //mV    // if Vcc < LOW_VCC give low voltage warning
#define ONE_MIN_MAX          12         // elements in the oneMinute accumulater array
#define LONG_PER_MAX  (10*ONE_MIN_MAX)  // elements in the longPeriod accumulater array (pegged to 10x ONE_MIN_MAX)
#define ONE_SEC_MAX          20         // elements in the oneSecond accumulator array
#define TIMERBASE_RC5      1778         // 1 bit time of RC5 element
#define SILENCE_ALARM_PERIOD 30000      // mS the alarm will be silenced for
#define DEBOUNCE_MS          50         // buttom debounce period in mS
#define POT_HYSTERESIS       4          // amount of change in potentiometer value needed to trigger a recalculation of toneSensitivity (keeps the value from bouncing around)
#define TONE_MIN_FREQ        31         // minimum frequency that will be generated by the CPMtoTone function - 31Hz is the lowest supported by the Arduino @16MHz
#define TONE_MAX_FREQ        4000       // maximum frequency that will be generated by the CPMtoTone function
#define AVGBGRAD_uSv         0.27       // global average background radiation level in uSv/h
#define AVBGRAD_uR           10.388     // global average background radiation level in uR/h
#define AVBGRAD_mR           0.010388   // global average background radiation level in mR/h
#define INFINITY             65534      // if scalerPeriod is set to this value, it will just do a cumulative count forever

//----------------------------------------------------------------------------------------------+
//                           Menu configuration parameters 
//----------------------------------------------------------------------------------------------+

// Configuration parameter minimum/maximum settings (where applicable)
#define DISP_PERIOD_MIN      1500       // probably shouldn't refresh more than every 1500ms
#define DISP_PERIOD_MAX      60000      // maximum display interval
#define LOGGING_PERIOD_MAX   43200      // maximum value in seconds for the logging interval (12 hours)
#define DOSE_RATIO_MAX       20000      // maximum value for the CPM to dose unit conversion ratio
#define MAX_ALARM            60000      // max the alarm can be set for
#define TONE_MAX_SENS        1000       // maximum value for tone sensitivity (max value = lowest possible sensitivity)
#define BARGRAPH_SCALE_MIN   2          // minimum value of the full scale CPM for the bargraph
#define BARGRAPH_SCALE_MAX   60000      // maximum value of the full scale CPM for the bargraph
#define SCALER_PER_MIN       2          // minimum allowed value for the long period scaler
#define SCALER_PER_MAX       90         // maximum allowed value for the long period scaler

// These are DEFAULTS! - only used if menu has not been run
#define DISP_PERIOD     5000            // defaults to 5 sec sample & display
#define LOGGING_PERIOD    60            // defaults a 60 sec logging period
#define PRI_RATIO        318            // defaults to SBT-11 ratio
#define SEC_RATIO        318            // TUBE_SEL jumper to GND - SET FOR LND712
#define ALARM_POINT      500            // CPM for Alarm defaults to
#define SCALER_PERIOD     10            // default scaler period
#define FULL_SCALE      1000            // max CPM for all 6 bars 
#define TONE_SENSITIVITY   4            // default tone sensitivity setting - controls how rapidly the tone increases in frequency as compared to CPS

// EEPROM Address for menu inputs
#define DISP_PERIOD_ADDR  0  // unsigned int - 2 bytes
#define LOG_PERIOD_ADDR   2  // unsigned int - 2 bytes
#define PRI_RATIO_ADDR    60 // float - 4 bytes
#define SEC_RATIO_ADDR    64 // float - 4 bytes
#define ALARM_SET_ADDR    6  // unsigned int - 2 bytes
#define DOSE_UNIT_ADDR    8 // byte - 1 byte 
#define ALARM_UNIT_ADDR   10 // boolean - 1 byte
#define SCALER_PER_ADDR   12 // unsigned int - 2 bytes
#define BARGRAPH_MAX_ADDR 14 // unsigned int - 2 bytes
#define TONE_SENS_ADDR    16 // unsigned int - 2 bytes
#define PIEZO_SET_ADDR    40 // boolean - 1 byte
#define SCALER_ADDR       42 // boolean - 1 byte  // not used - don't come up in scaler mode
#define RADLOGGER_ADDR    44 // boolean - 1 byte
#define ISPRIMARYRATE_ADDR    46 // boolean - 1 byte
#define ADDA_ADDR       0x4F // address for PCF8591 if all address lines pulled high

//----------------------------------------------------------------------------------------------+
//                                     Globals
//----------------------------------------------------------------------------------------------+
boolean toggle = false;
// These hold the local values that have been read from EEPROM
unsigned long LoggingPeriod;            // mS between writes to serial
float doseRatio;                        // holds the rate selected by jumper
unsigned int AlarmPoint;                // alarm if > than this setting

boolean scalerParam;                    // flag indicating whether the scaler screen should be on or off
boolean radLogger;                      // flag indicating whether the Radiation Logger serial output is used
boolean isPrimary;                      // flag indicating which CPM to dose rate is currently used
unsigned int scalerPeriod;              // period for the > 1 minute scaler
byte doseUnit;                          // 0 - uSv/H, 1 - uR/H, 2 - mR/H
boolean alarmInCPM;                     // 1 = CPM, 0 - DisplayUnit

boolean lowVcc = false;                 // true when Vcc < LOW_VCC
//boolean Backlight = true;             // preset to backlight = ON
boolean PiezoOn = false;                 // preset to piezo = ON
// variables for counting periods and counts . . .
unsigned long dispPeriodStart, dispPeriod; // for display period
unsigned long dispCnt;                  // to count and display CPM
bool isfirepulse = false;               // if count event is detected this variable is set to true and will instruct the routine FirePulse() to produce a single pulse on PULSE_PIN digital pin 19 (A5)
boolean AlarmOn = false;                // CPM > set alarm
boolean alarmSilence = false;           // true if alarm has been silenced
int androidReturn = 0;                  // reads the slider on Android SensorGraph.

volatile unsigned long fastCnt;
unsigned long logPeriodStart;           // for logging period
volatile unsigned long logCnt;          // to count and log CPM
unsigned long fastCountStart;           // counter for bargraph refresh period
unsigned int bargraphMax;

unsigned long oneMinCountStart;         // timer for running average
volatile unsigned long oneMinCnt;       // counter for running averages
volatile unsigned long longPeriodCnt;   // counter for the long period scaler
unsigned long longPeriodStart;          // start time for long period scaler
unsigned long alarmSilenceStart;        // timer for silencing the alarm is Select pressed

unsigned long oneMinute[ONE_MIN_MAX];   // array holding counts for 1 minute running average
unsigned long longPeriod[LONG_PER_MAX]; // array holding counts for 10 minute running average
unsigned long oneSecond[ONE_SEC_MAX];   // array holding counts for 1 second running average
byte oneMinuteIndex = 0;                // index to 1 minute array
byte longPeriodIndex = 0;               // index to 10 minute array

boolean dispOneMin = false;             // indicates 1 minute average is available
boolean dispLongPeriod = false;         // indicates 10 minute average is available
unsigned long nullPoint;                // zero point for tone mode
unsigned int toneSensitivity;           // controls how rapidly pitch increases as a function of CPS
boolean setNullPoint = false;           // when set true, causes the null point to be set to the displayed CPM
unsigned long currentDispCPM = 0;       // holds the current CPM value on the display - used with the button to set the null point
/////////////////////////////// ADDED FOR METER //////////////////////////////
unsigned long lastVCC;                  // MAKE ONLY 1 CALL TO readVcc FOR EFFICENCY - TO FINISH
#if (ANALOG_METER)
byte digIn[5];                          // holds 4 bytes of input from PCF8591 (1 based)
#define NUM_KEYS    5                   // number of rotary switch positions (max 5)
int  adc_key_val[5] ={
  20, 45, 100, 140, 200 };              // ADC return must be UNDER these values
#endif

//----------------------------------------------------------------------------------------------+
//                                    For menu
//----------------------------------------------------------------------------------------------+

#define MAX_MENU  10
#define MENU_DISP_PER     0
#define MENU_LOG_PER      1
#define MENU_RATIO        2
#define MENU_ALARM        3
#define MENU_DOSE_UNIT    4
#define MENU_ALARM_UNIT   5
#define MENU_SCALER_PER   6
#define MENU_BARGRAPH_MAX 7
#define MENU_RADLOGGER    8
#define MENU_TONE_SENS    9
#define MENU_BATT         10

//const prog_char string_0[] PROGMEM = "DISP PERIOD (MS)";   // prog_char "String 0" etc are strings to store - change to suit.
const char string_0[] PROGMEM = "DISP PERIOD (MS)";   // prog_char "String 0" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "LOG PERIOD";
const char string_2[] PROGMEM = "CPM->      RATIO";   // spaces will be filled with actual unit when displayed
const char string_3[] PROGMEM = "ALARM THRESHOLD";
const char string_4[] PROGMEM = "DOSE UNIT";
const char string_5[] PROGMEM = "ALARM UNIT";
const char string_6[] PROGMEM = "SCALER PER (MIN)";
const char string_7[] PROGMEM = "BARGRAPH MAX CPM";
const char string_8[] PROGMEM = "USE RADLOGGER?";
const char string_9[] PROGMEM = "TONE SENSITIVITY";
const char string_10[] PROGMEM = "REG. VOLTAGE";

//buvo PROGMEM const char *menu_table[] = 	// PROGMEM array to hold MENU strings
const char* const menu_table[] PROGMEM = {	// PROGMEM array to hold MENU strings
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6,
  string_7,
  string_8,
  string_9,
  string_10
};

#define MAX_UNIT  2
// unit strings for LCD - use the LCD's built in char table to get mu char instead of u where applicable
#if (DOGM_LCD) // TODO - Can't figure out how to print mu - it prints ¢
//prog_char unit_lcd_0[] PROGMEM = "uSv/h";
const char unit_lcd_0[] PROGMEM = "uSv/h";
const char unit_lcd_1[] PROGMEM = "u4R/h";
const char unit_lcd_2[] PROGMEM = "mR/h";
#else
const char unit_lcd_0[] PROGMEM = "\xe4Sv/h";
const char unit_lcd_1[] PROGMEM = "\xe4R/h";
const char unit_lcd_2[] PROGMEM = "mR/h";
#endif
//buvo PROGMEM const char *unit_lcd_table[] = { // PROGMEM array to hold unit strings
const char* const unit_lcd_table[] PROGMEM = { // PROGMEM array to hold unit strings
  unit_lcd_0,
  unit_lcd_1,
  unit_lcd_2
};

// unit strings used for logging - use u instead of mu since nobody interprets chars above 127 consistently
const char unit_0[] PROGMEM = "uSv/h";
const char unit_1[] PROGMEM = "uR/h";
const char unit_2[] PROGMEM = "mR/h";

//buvo PROGMEM const char *unit_table[] = { // PROGMEM array to hold unit strings
const char* const unit_table[] PROGMEM = { // PROGMEM array to hold unit strings
 // buvo unit_0,
  unit_0,
  unit_1,
  unit_2
};

//----------------------------------------------------------------------------------------------+
//                                    Custom Characters for Bargraph
//----------------------------------------------------------------------------------------------+


#if (EIGHT_CHAR)
//No Bar Graph - custom characters used for icons to save chars
byte cpmIcon[8] = {
  0x18, 0x11, 0x1A, 0x04, 0x00, 0x1B, 0x15, 0x11}; // "CPM" icon
byte usvIcon[8] = {
  0x14, 0x1C, 0x00, 0x18, 0x10, 0x0D, 0x1A, 0x00}; // "uSv" icon
byte oneIcon[8] = {
  0x0C, 0x04, 0x04, 0x0E, 0x00, 0x1B, 0x15, 0x11}; // 1 minute icon
byte tenIcon[8] = {
  0x17, 0x15, 0x17, 0x00, 0x00, 0x1B, 0x15, 0x11}; // 10 minute icon
byte cpm2Icon[8] = {
  0x18, 0x11, 0x1A, 0x04, 0x00, 0x1B, 0x15, 0x11}; // "CPM" icon again (Serial.write(0) no work
#else

// custom characters used for bar graph

// blank
byte bar_0[8] = {        
  B00000,
  B00000,
  B00000,
  B10101,
  B00000,
  B00000,
  B00000,
  B00000
}; 

// 1 bar
byte bar_1[8] = {
  B00000,
  B10000,
  B10000,
  B10101,
  B10000,
  B10000,
  B00000,
  B00000
};

// 2 bars
byte bar_2[8] = {
  B00000,
  B11000,
  B11000,
  B11101,
  B11000,
  B11000,
  B00000,
  B00000
};

// 3 bars
byte bar_3[8] = {
  B00000,
  B11100,
  B11100,
  B11101,
  B11100,
  B11100,
  B00000,
  B00000
};

// 4 bars
byte bar_4[8] = {
  B00000,
  B11110,
  B11110,
  B11111,
  B11110,
  B11110,
  B00000,
  B00000
};

// 5 bars
byte bar_5[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000
};
#endif
