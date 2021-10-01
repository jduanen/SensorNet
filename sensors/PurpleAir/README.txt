# SensorNet PurpleAir proxy client service

* set up 
  - sudo cp ~/Code/SensorNet/sensors/PurpleAir/purpleair.service /lib/systemd/system
  - sudo ln -s /lib/systemd/system/purpleair.service /etc/systemd/system/
  - sudo systemctl daemon-reload

* start service
  - sudo systemctl start purpleair.service

* check status of service
  - sudo systemctl status purpleair.service

* enable the service on every reboot
  - sudo systemctl enable purpleair.service

* disable your service on every reboot
  - sudo systemctl disable purpleair.service
