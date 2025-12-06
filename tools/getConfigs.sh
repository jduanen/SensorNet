#!/bin/bash
#
# Script to copy ESPHome config files from their github locations to the local dir
#

SOURCE="/home/jdn/Code/SensorNet"
CONTROLLERS="${SOURCE}/controllers"
SENSORS="${SOURCE}/sensors"
VOICE_ASSISTANTS="${SOURCE}/voiceAssistants"

echo "Copying..."


# **Controllers
# Feeder Door
yamlFiles="feeder-door.yaml"
cp ${CONTROLLERS}/FeederDoor/${yamlFiles} .


# **Sensors
# Air Quality Sensors (PMS)
yamlFiles="air-quality-pms-0.yaml air-quality-pms-1.yaml air-quality-pms-2.yaml"
for f in ${yamlFiles}; do
  cp ${SENSORS}/AirQualityPMS/${f} .
done

# Air Quality Sensors (SPS)
yamlFiles="air-quality-sps-0.yaml"
cp ${SENSORS}/AirQualitySPS/${yamlFiles} .

# Environmental Sensors
yamlFiles="env-sensors.yaml"
cp ${SENSORS}/EnvironmentalSensors/${yamlFiles} .

# KittyCams
yamlFiles="kittycam.yaml"
cp ${SENSORS}/KittyCam/${yamlFiles} .

yamlFiles="kittycam2.yaml"
cp ${SENSORS}/KittyCamV2/${yamlFiles} .

# LED Sign
yamlFiles="led-sign.yaml"
cp ${SENSORS}/LedSign/${yamlFiles} .

# Radiation Sensors
yamlFiles="radiation-0.yaml radiation-1.yaml"
for f in ${yamlFiles}; do
    cp ${SENSORS}/Radiation/${f} .
done

# Smart Plugs (EMS01)
yamlFiles="smart-plug-ems01-0.yaml"
cp ${SENSORS}/SmartPlugEMS01/${yamlFiles} .

# Temperature Sensor with Display
yamlFiles="tempsense0.yaml"
cp ${SENSORS}/TemperatureDisplay/${yamlFiles} .

# Temperature Sensor
yamlFiles="tempsense1.yaml"
cp ${SENSORS}/Temperature/${yamlFiles} .

# Water Heater Temperature Sensor
yamlFiles="water-temperature.yaml"
cp ${SENSORS}/WaterHeater/${yamlFiles} .

# Water Heater Leak Detector
yamlFiles="waterheater-leak-detector.yaml"
cp ${SENSORS}/WaterHeaterLeak/${yamlFiles} .


# **Voice Assistants
# Home Assistant Voice
yamlFiles="home-assistant-voice-0.yaml"
cp ${VOICE_ASSISTANTS}/HomeAssistantVoicePE/${yamlFiles} .

# Respeaker XVF3800 Satellite
yamlFiles="respeaker-xvf3800-0.yaml"
cp ${VOICE_ASSISTANTS}/ReSpeakerVXF3800Satellite/${yamlFiles} .

# Waveshare Satellites
yamlFiles="waveshare-audio-0.yaml waveshare-audio-1.yaml"
for f in ${yamlFiles}; do
    cp ${VOICE_ASSISTANTS}/WaveshareSatellite/${f} .
done

# Finish up
#### TODO print any failures (after making this detect failures and go on)

unset SSHPASS
echo "Done copying"
