#!/bin/bash
#
# Script to push ESPHome config files from HA Server to gpuServer1.lan
#
# Create key on HA server first:
#  ssh-keygen -t ed25519 -f ~/.ssh/rsyncKey -C "rsync automation key"
#  chmod 600 ~/.ssh/rsyncKey
#  chmod 644 ~/.ssh/rsyncKey.pub
#  ssh-copy-id -i ~/.ssh/rsyncKey.pub jdn@gpuServer1.local
# Test with:
#  ssh -i /root/.ssh/rsyncKey jdn@gpuServer1.local

#### TODO move this to a common file and include it

REMOTE_USER_HOST="jdn@gpuServer1.local"
REMOTE_DIR="Code/SensorNet"

LOCAL_DIR="/root/config/esphome"

CONTROLLERS="${REMOTE_DIR}/controllers"
SENSORS="${REMOTE_DIR}/sensors"
DISPLAYS="${SOURCE_DIR}/displays"
VOICE_ASSISTANTS="${REMOTE_DIR}/voiceAssistants"

REMOTE_FILES=(
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

KEY="~/.ssh/rsyncKey"

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

for remoteFile in "${REMOTE_FILES[@]}"; do
    localFile="$LOCAL_DIR/${remoteFile##*/}"
    rsync -avqz -e "ssh -i $KEY" --protect-args "$localFile" "$REMOTE_USER_HOST:$remoteFile"
done
