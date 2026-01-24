#!/bin/bash
#
# Script to compare ESPHome config pulled from the HA Server with their source files
#
# To be run on gpuServer1

VERBOSE=${VERBOSE:-}  # defaults to empty if not set on command line
COPY_FILES=${COPY_FILES:-}  # if set on command line, then copy changed files

SOURCE_DIR="${HOME}/Code/SensorNet"
CONFIG_DIR="${SOURCE_DIR}/config/esphome"

CONTROLLERS="${SOURCE_DIR}/controllers"
SENSORS="${SOURCE_DIR}/sensors"
DISPLAYS="${SOURCE_DIR}/displays"
VOICE_ASSISTANTS="${SOURCE_DIR}/voiceAssistants"

compareFile() {
    local localFile="$1" remoteFile="$2"
    if [[ -f "$localFile" ]]; then
        if [[ -f "$remoteFile" ]]; then
            if diff "$localFile" "$remoteFile" >/dev/null 2>&1; then
                if [[ -n "$VERBOSE" ]]; then
                    echo "SAME: $localFile == $remoteFile"
                    echo ""
                fi
            else
                echo "DIFF:"
                echo "$localFile != $remoteFile"
                if [[ $VERBOSE -gt 1 ]]; then
                    diff "$localFile" "$remoteFile"
                fi
                if [[ -n "$COPY_FILES" ]]; then
                    cp "$localFile" "$remoteFile"
                    if [[ -n "$VERBOSE" ]]; then
                        echo "Copied changed file to: $remoteFile"
                    fi
                fi
                echo ""
            fi
        else
            echo "WARNING: Remote file not found ($remoteFile)"
            echo ""
        fi
    else
        echo "WARNING: Local file not found ($localFile)"
        echo ""
    fi
}

# **Controllers
# Feeder Door
yamlFile="feeder-door.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${CONTROLLERS}/FeederDoor/${yamlFile}

# **Sensors
# Air Quality Sensors (PMS)
yamlFiles="air-quality-pms-0.yaml air-quality-pms-1.yaml air-quality-pms-2.yaml"
for f in $yamlFiles; do
  compareFile "$CONFIG_DIR/$f" ${SENSORS}/AirQualityPMS/${f}
done

# Air Quality Sensors (SPS)
yamlFile="air-quality-sps-0.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/AirQualitySPS/${yamlFile}

# Environmental Sensors
yamlFile="env-sensors.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/EnvironmentalSensors/${yamlFile}

# KittyCams
yamlFile="kittycam.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/KittyCam/${yamlFile}

yamlFile="kittycam2.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/KittyCamV2/${yamlFile}

# Radiation Sensors
yamlFiles="radiation-0.yaml radiation-1.yaml"
for f in ${yamlFiles}; do
    compareFile "$CONFIG_DIR/$f" ${SENSORS}/Radiation/${f}
done

# Smart Plugs (EMS01)
yamlFile="smart-plug-ems01-0.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/SmartPlugEMS01/${yamlFile}

# Temperature Sensor with Display
yamlFile="tempsense0.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/TemperatureDisplay/${yamlFile}

# Temperature Sensor
yamlFile="tempsense1.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/Temperature/${yamlFile}

# Water Heater Temperature Sensor
yamlFile="water-temperature.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/WaterHeater/${yamlFile}

# Water Heater Leak Detector
yamlFile="waterheater-leak-detector.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${SENSORS}/WaterHeaterLeak/${yamlFile}

# **Displays
# LED Sign
yamlFile="led-sign.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${DISPLAYS}/LedSign/${yamlFile}

# **Voice Assistants
# Home Assistant Voice
yamlFile="home-assistant-voice-0.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${VOICE_ASSISTANTS}/HomeAssistantVoicePE/${yamlFile}

# Respeaker XVF3800 Satellite
yamlFile="respeaker-xvf3800-0.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${VOICE_ASSISTANTS}/ReSpeakerVXF3800Satellite/${yamlFile}

# FutureProofHomes Satellite1
yamlFile="satellite1-cfec40.yaml"
compareFile "$CONFIG_DIR/$yamlFile" ${VOICE_ASSISTANTS}/Satellite1/${yamlFile}

# Waveshare Satellites
yamlFiles="waveshare-audio-0.yaml waveshare-audio-1.yaml"
for f in ${yamlFiles}; do
    compareFile "$CONFIG_DIR/$yamlFile" ${VOICE_ASSISTANTS}/WaveshareSatellite/${f}
done
