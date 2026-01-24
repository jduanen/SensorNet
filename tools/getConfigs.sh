#!/bin/bash
#
# Script to copy ESPHome config files from their github locations to the local dir
#

SOURCE="/home/jdn/Code/SensorNet"
CONTROLLERS="${SOURCE}/controllers"
SENSORS="${SOURCE}/sensors"
DISPLAYS="${SOURCE_DIR}/displays"
VOICE_ASSISTANTS="${SOURCE}/voiceAssistants"

FAILED=

copyFile() {
    f="$1"
    ##echo "trying: $f"
    cp "$f" . >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        b=${f##*/}
        if [ -z "$FAILED" ]; then
            FAILED="$b"
        else
            FAILED="${FAILED} $b"
        fi
    fi
}

echo "Copying..."


# **Controllers
# Feeder Door
yamlFiles="feeder-door.yaml"
copyFile ${CONTROLLERS}/FeederDoor/${yamlFiles}


# **Sensors
# Air Quality Sensors (PMS)
yamlFiles="air-quality-pms-0.yaml air-quality-pms-1.yaml air-quality-pms-2.yaml"
for f in ${yamlFiles}; do
  copyFile ${SENSORS}/AirQualityPMS/${f}
done

# Air Quality Sensors (SPS)
yamlFiles="air-quality-sps-0.yaml"
copyFile ${SENSORS}/AirQualitySPS/${yamlFiles}

# Environmental Sensors
yamlFiles="env-sensors.yaml"
copyFile ${SENSORS}/EnvironmentalSensors/${yamlFiles}

# KittyCams
yamlFiles="kittycam.yaml"
copyFile ${SENSORS}/KittyCam/${yamlFiles}

yamlFiles="kittycam2.yaml"
copyFile ${SENSORS}/KittyCamV2/${yamlFiles}

# Radiation Sensors
yamlFiles="radiation-0.yaml radiation-1.yaml"
for f in ${yamlFiles}; do
    copyFile ${SENSORS}/Radiation/${f}
done

# Smart Plugs (EMS01)
yamlFiles="smart-plug-ems01-0.yaml"
copyFile ${SENSORS}/SmartPlugEMS01/${yamlFiles}

# Temperature Sensor with Display
yamlFiles="tempsense0.yaml"
copyFile ${SENSORS}/TemperatureDisplay/${yamlFiles}

# Temperature Sensor
yamlFiles="tempsense1.yaml"
copyFile ${SENSORS}/Temperature/${yamlFiles}

# Water Heater Temperature Sensor
yamlFiles="water-temperature.yaml"
copyFile ${SENSORS}/WaterHeater/${yamlFiles}

# Water Heater Leak Detector
yamlFiles="waterheater-leak-detector.yaml"
copyFile ${SENSORS}/WaterHeaterLeak/${yamlFiles}

# **Displays
# LED Sign
yamlFiles="led-sign.yaml"
copyFile ${SENSORS}/LedSign/${yamlFiles}

# **Voice Assistants
# Home Assistant Voice
yamlFiles="home-assistant-voice-0.yaml"
copyFile ${VOICE_ASSISTANTS}/HomeAssistantVoicePE/${yamlFiles}

# Respeaker XVF3800 Satellite
yamlFiles="respeaker-xvf3800-0.yaml"
copyFile ${VOICE_ASSISTANTS}/ReSpeakerVXF3800Satellite/${yamlFiles}

# Waveshare Satellites
yamlFiles="waveshare-audio-0.yaml waveshare-audio-1.yaml"
for f in ${yamlFiles}; do
    copyFile ${VOICE_ASSISTANTS}/WaveshareSatellite/${f}
done

# Finish up
if [ -z "$FAILED" ]; then
    echo "    All Succeeded"
    r=0
else
    echo "    Failed: $FAILED"
    r=1
fi

unset SSHPASS
echo "Done copying"

exit $r
