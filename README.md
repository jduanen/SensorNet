# SensorNet

Collection of ESP8266-based sensors that generate data via MQTT and a suite of tools for operating on the data

## Sensors

### AirQuality

Measure airborne particulate matter of different sizes.

#### Sensirion SPS30 (SPS)

*TBD*

#### Plantower PMS7003 (PMS)

*TBD*

### Radiation (Radiation)

Measure alpha, beta, gamma radiation.

*TBD*

### WaterHeater (WaterHeater)

Measure temperature at the outlet of water heater and ambient temperature.

*TBD*

### Hummingbird Feeder Scale (BirdyNomNom)

Measure weight of sugar water in hummingbird feeder.

*TBD*

### Avue PZT Camera Platform (avue)

Monitor the internal temperature and WiFi signal strength of my Avue PZT camera controller ([AvuePZT](https://github.com/jduanen/avuePZT)).

*TBD*

## Notes

### Topics

* /sensors
    - /sensors/AirQuality/
        + /sensors/AirQuality/PMS/*WifiMAC*
            * latest version: 1.1.0
            * /sensors/AirQuality/PMS/*WifiMAC*/data
                - payload:
                    * pm1_0:d: ????
                    * pm2_5:d: ????
                    * pm10_0:d: ????
            * /sensors/AirQuality/PMS/*WifiMAC*/cmd
                - commands:
                    + *TBD*
        + /sensors/AirQuality/SPS/*WifiMAC*
            * latest version: 1.0.2
            * /sensors/AirQuality/SPS/*WifiMAC*/data
                - payload:
                    * pm1_0:.2f: ????
                    * pm2_5:.2f: ????
                    * pm4_0:.2f: ????
                    * pm10_0:.2f: ????
                    * nc0_5:.2f: ????
                    * nc1_0:.2f: ????
                    * nc2_5:.2f: ????
                    * nc4_0:.2f: ????
                    * nc10_0:.2f: ????
                    * tps:.2f: ????
            * /sensors/AirQuality/SPS/*WifiMAC*/cmd
                - commands:
                    + *TBD*
    - /sensors/birdyNomNom/*WifiMAC*
        + /sensors/birdyNomNom/*WifiMAC*/data
        + /sensors/birdyNomNom/*WifiMAC*/cmd
    - /sensors/Radiation/*WifiMAC*
        + latest version: 1.0.2
        + /sensors/Radiation/*WifiMAC*/data
            * payload:
                - CPM:d: ????
                - uSv/h:.4f: ????
                - Vcc:.2f: ????
        + /sensors/Radiation/*WifiMAC*/cmd
            * commands:
                - *TBD*
    - /sensors/WaterHeater/*WifiMAC*
        + latest version: 0.0.0
        + /sensors/WaterHeater/*WifiMAC*/data
            * payload:
                - waterDegC:3.2f: ????
                - ambientDegC:3.2f: ????
        + /sensors/WaterHeater/*WifiMAC*/cmd
            * commands:
                - *TBD*
    - /sensors/avue/*WifiMAC*
        + latest version: 1.1.0
        + /sensors/avue/*WifiMAC*/data
            * payload:
                - temp:.1f: ????
                - q:.4f: ????
                - rssi:d: ????
        + /sensors/avue/*WifiMAC*/cmd
            * commands:
                - *TBD*

### Startup Lines

* AirQuality (PMS)
    - *ISO timestamp*,/sensors/AirQuality/PMS/*WifiMAC*/cmd,Startup,ESP8266,AirQualityPMS,1.1.0,pm1_0:d,pm2_5:d,pm10_0:d,*RSSI*
* AirQuality (SPS)
    - *ISO timestamp*,/sensors/AirQuality/SPS/*WifiMAC*/cmd,Startup,ESP8266,AirQualitySPS,1.0.2,pm1_0:.2f,pm2_5:.2f,pm4_0:.2f,pm10_0:.2f,nc0_5:.2f,nc1_0:.2f,nc2_5:.2f,nc4_0:.2f,nc10_0:.2f,tps:.2f,*RSSI*
* Radiation
    - *ISO timestamp*,/sensors/Radiation/*WifiMAC*/cmd,Startup,ESP8266,Radiation,1.0.2,CPM:d,uSv/h:.4f,Vcc:.2f,*RSSI*
* WaterHeater
    - *ISO timestamp*,/sensors/WaterHeater/*WifiMAC*/cmd,Startup,ESP8266,WaterHeater,0.0.0,waterDegC:3.2f,ambientDegC:3.2f,*RSSI*
* Avue PZT Camera
    - *ISO timestamp*,/sensors/avue/*WifiMAC*/cmd,Startup,Rpi3,AvuePZT,1.1.0,temp:.1f,q:.4f,rssi:d,*RSSI*


## TODO

* write test scripts
* create schematics for all the sensors
* document code
* make callbacks work for subs in SensorNet library
* make SensorNet a proper library
  - create kewords.txt and library.properties
  - add a README.md and LICENSE
  - create tests in examples/
  - document interface
