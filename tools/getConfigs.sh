#!/bin/bash
#
# Script to copy ESPHome config files from their github locations to the local dir
#

SOURCE="/home/jdn/Code/SensorNet/sensors"

echo "Copying..."

# Air Quality Sensors (PMS)
yamlFiles="air-quality-pms-0.yaml air-quality-pms-1.yaml air-quality-pms-2.yaml"
for f in ${yamlFiles}; do
  cp ${SOURCE}/AirQualityPMS/${f} .
done

# Air Quality Sensors (SPS)
yamlFiles="air-quality-sps-0.yaml"
cp ${SOURCE}/AirQualitySPS/${yamlFiles} .

# Environmental Sensors
yamlFiles="env-sensors.yaml"
cp ${SOURCE}/EnvironmentalSensors/${yamlFiles} .

# Feeder Door
yamlFiles="feeder-door.yaml"
cp ${SOURCE}/FeederDoor/${yamlFiles} .

# Home Assistant Voice
yamlFiles="home-assistant-voice-0.yaml"
cp ${SOURCE}/HomeAssistantVoicePE/${yamlFiles} .

# KittyCams
yamlFiles="kittycam.yaml"
cp ${SOURCE}/KittyCam/${yamlFiles} .

yamlFiles="kittycam2.yaml"
cp ${SOURCE}/KittyCamV2/${yamlFiles} .

# LED Sign
yamlFiles="led-sign.yaml"
cp ${SOURCE}/LedDisplay/${yamlFiles} .

# Radiation Sensors
yamlFiles="radiation-0.yaml radiation-1.yaml"
for f in ${yamlFiles}; do
    cp ${SOURCE}/Radiation/${f} .
done

# Respeaker XVF3800 Satellite
yamlFiles="respeaker-xvf3800-0.yaml"
cp ${SOURCE}/ReSpeakerVXF3800Satellite/${yamlFiles} .

# Smart Plugs (EMS01)
yamlFiles="smart-plug-ems01-0.yaml"
cp ${SOURCE}/SmartPlugEMS01/${yamlFiles} .

# Temperature Sensor with Display
yamlFiles="tempsense0.yaml"
cp ${SOURCE}/TemperatureDisplay/${yamlFiles} .

# Temperature Sensor
yamlFiles="tempsense1.yaml"
cp ${SOURCE}/Temperature/${yamlFiles} .

# Water Heater Temperature Sensor
yamlFiles="water-temperature.yaml"
cp ${SOURCE}/WaterHeater/${yamlFiles} .

# Water Heater Leak Detector
yamlFiles="waterheater-leak-detector.yaml"
cp ${SOURCE}/WaterHeaterLeak/${yamlFiles} .

# Waveshare Satellites
yamlFiles="waveshare-audio-0.yaml waveshare-audio-1.yaml"
for f in ${yamlFiles}; do
    cp ${SOURCE}/WaveshareSatellite/${f} .
done

# Finish up
#### TODO print any failures (after making this detect failures and go on)

unset SSHPASS
echo "Done copying"
