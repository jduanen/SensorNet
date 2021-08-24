#!/bin/bash
#
# Run logger on startup by ~/.config/autostart/sensornet.desktop
#

workon SensorNet

/home/jdn/Code/SensorNet/tools/MqttLogger/MqttLogger.py -v -a
