SensorNet

#Collection of ESP8266-based sensors that generate data via MQTT and a suite of tools for operating on the data

##Topics

* /sensors
    - /sensors/airQuality/
        + /sensors/airQuality/<WifiMAC>
            * /sensors/airQuality/<WifiMAC>/data
            * /sensors/airQuality/<WifiMAC>/cmd
    - /sensors/birdyNomNom/
        + /sensors/birdyNomNom/<WifiMAC>
            * /sensors/birdyNomNom/<WifiMAC>/data
            * /sensors/birdyNomNom/<WifiMAC>/cmd
    - /sensors/radiation/
        + /sensors/radiation/<WifiMAC>
            * /sensors/radiation/<WifiMAC>/data
            * /sensors/radiation/<WifiMAC>/cmd

## TODO

* write test scripts
* document code
* make callbacks work for subs in SensorNet library
* make SensorNet a proper library
  - create kewords.txt and library.properties
  - add a README.md and LICENSE
  - create tests in examples/
  - document interface
