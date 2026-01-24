#!/bin/bash
#
# Script to validate that all of the config files pass syntax checks
#
# Have to be in the ESPHOME venv
#  workon ESPHOME
# Also needs to have symlink to secrets.yaml

#### TODO move this to a common file and include it

LOG_FILE="/tmp/sensornet.log"
SOURCE_DIR="${HOME}/Code/SensorNet"

CONTROLLERS="${SOURCE_DIR}/controllers"
SENSORS="${SOURCE_DIR}/sensors"
DISPLAYS="${SOURCE_DIR}/displays"
VOICE_ASSISTANTS="${SOURCE_DIR}/voiceAssistants"

SOURCE_FILES=(
    "${CONTROLLERS}/FeederDoor/feeder-door.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-0.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-1.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-2.yaml"
    "${SENSORS}/AirQualitySPS/air-quality-sps-0.yaml"
    "${SENSORS}/EnvironmentalSensors/env-sensors.yaml"
    "${SENSORS}/KittyCam/kittycam.yaml"
    "${SENSORS}/KittyCamV2/kittycam2.yaml"
    "${SENSORS}/Radiation/radiation-0.yaml"
    "${SENSORS}/Radiation/radiation-1.yaml"
    "${SENSORS}/SmartPlugEMS01/smart-plug-ems01-0.yaml"
    "${SENSORS}/TemperatureDisplay/tempsense0.yaml"
    "${SENSORS}/Temperature/tempsense1.yaml"
    "${SENSORS}/WaterHeater/water-temperature.yaml"
    "${SENSORS}/WaterHeaterLeak/waterheater-leak-detector.yaml"
    "${DISPLAYS}/LedSign/led-sign.yaml"
    "${VOICE_ASSISTANTS}/HomeAssistantVoicePE/home-assistant-voice-0.yaml"
    "${VOICE_ASSISTANTS}/ReSpeakerVXF3800Satellite/respeaker-xvf3800-0.yaml"
    "${VOICE_ASSISTANTS}/Satellite1/satellite1-cfec40.yaml"
    "${VOICE_ASSISTANTS}/WaveshareSatellite/waveshare-audio-0.yaml"
    "${VOICE_ASSISTANTS}/WaveshareSatellite/waveshare-audio-1.yaml"
)

SUCCEEDED=
FAILED=

date > $LOG_FILE
echo "----------------" >> $LOG_FILE

for file in "${SOURCE_FILES[@]}"; do
    if [[ -f "$file" ]]; then
        if [[ -n "$VERBOSE" ]]; then
            echo -n "    Validating: $file ... "
        fi
        esphome -q config $file &>> $LOG_FILE
        if [[ $? -ne 0 ]]; then
            if [[ -n "$VERBOSE" ]]; then
                echo "Failed."
            fi
            if [[ -z "$FAILED" ]]; then
                FAILED=$file
            else
                FAILED="$FAILED $file"
            fi
            echo "^^^^^^^^ $file ^^^^^^^^" >> $LOG_FILE
        else
            if [[ -n "$VERBOSE" ]]; then
                echo "Succeeded."
            fi
            if [[ -z "$SUCCEEDED" ]]; then
                SUCCEEDED=$file
            else
                SUCCEEDED="$SUCCEEDED $file"
            fi
        fi
    else
        echo "ERROR: missing file ($file)"
        exit 1
    fi
done

echo ""
if [[ -n $SUCCEEDED ]]; then
    echo "Succeeded: "
    for s in $SUCCEEDED; do
        echo "    "$s
    done
    echo ""
fi
if [[ -n $FAILED ]]; then
    echo "Failed: "
    for f in $FAILED; do
        echo "    "$f
    done
    echo "See logs in: $LOG_FILE"
fi
