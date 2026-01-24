#!/bin/bash
#
# Common code that defines the config files to be used by bash scripts in this directory
#

REMOTE_USER_HOST="jdn@gpuServer1.local"
REMOTE_DIR="Code/SensorNet"

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
