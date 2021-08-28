# MQTT logging service (for Ubuntu)

* set up 
  - sudo cp ~/Code/SensorNet/tools/MqttLogger/mqttlogger.service /lib/systemd/system
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
