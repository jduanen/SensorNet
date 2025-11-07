# SensorNet tools package

## MQTT logging service (for Ubuntu)

* set up 
  - sudo cp ~/Code/SensorNet/tools/mqttlogger.service /lib/systemd/system
  - sudo ln -s /lib/systemd/system/mqttlogger.service /etc/systemd/system/
  - sudo systemctl daemon-reload

* start service
  - sudo systemctl start mqttlogger.service

* check status of service
  - sudo systemctl status mqttlogger.service

* enable the service on every reboot
  - sudo systemctl enable mqttlogger.service

* disable your service on every reboot
  - sudo systemctl disable mqttlogger.service

## SensorManager tool

* get/set sample interval
* get RSSI

## Sensor Monitor tool

* one-shot mode
  - report last event from all (or specified) devices
  - report all excursions outside of a given range for a specific application
* loop mode
  - report if haven't heard from given device(s) in specified amount of time
  - report if a given value for a specific application is outside of a specified range

## SensorNet Utlities library


## Sensor Data Plotting tool
