#!/bin/bash
#
# Script to pull ESPHome config files from gpuServer1.lan to HA Server
#
# Create key on HA server first:
#  ssh-keygen -t ed25519 -f ~/.ssh/rsyncKey -C "rsync automation key"
#  chmod 600 ~/.ssh/rsyncKey
#  chmod 644 ~/.ssh/rsyncKey.pub
#  ssh-copy-id -i ~/.ssh/rsyncKey.pub jdn@gpuServer1.local
# Test with:
#  ssh -i /root/.ssh/rsyncKey jdn@gpuServer1.local

REMOTE_DIR="jdn@gpuServer1.local:Code/SensorNet/config/esphome/"

LOCAL_DIR="/root/config/esphome"

CONTROLLERS="${SOURCE}/controllers"
SENSORS="${SOURCE}/sensors"
VOICE_ASSISTANTS="${SOURCE}/voiceAssistants"

REMOTE_FILES=(
    "${CONTROLLERS}/FeederDoor/feeder-door.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-0.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-1.yaml"
    "${SENSORS}/AirQualityPMS/air-quality-pms-2.yaml"
    "${SENSORS}/AirQualitySPS/air-quality-sps-0.yaml"
    "${SENSORS}/EnvironmentalSensors/env-sensors.yaml"
    "${SENSORS}/KittyCam/kittycam.yaml"
    "${SENSORS}/KittyCamV2/kittycam2.yaml"
    "${SENSORS}/LedSign/led-sign.yaml"
    "${SENSORS}/Radiation/radiation-0.yaml"
    "${SENSORS}/Radiation/radiation-1.yaml"
    "${SENSORS}/SmartPlugEMS01/smart-plug-ems01-0.yaml"
    "${SENSORS}/TemperatureDisplay/tempsense0.yaml"
    "${SENSORS}/Temperature/tempsense1.yaml"
    "${SENSORS}/WaterHeater/water-temperature.yaml"
    "${SENSORS}/WaterHeaterLeak/waterheater-leak-detector.yaml"
    "${VOICE_ASSISTANTS}/HomeAssistantVoicePE/home-assistant-voice-0.yaml"
    "${VOICE_ASSISTANTS}/ReSpeakerVXF3800Satellite/respeaker-xvf3800-0.yaml"
    "${VOICE_ASSISTANTS}/Satellite1/satellite1-cfec40.yaml"
    "${VOICE_ASSISTANTS}/WaveshareSatellite/waveshare-audio-0.yaml"
    "${VOICE_ASSISTANTS}/WaveshareSatellite/waveshare-audio-1.yaml"
)

KEY="~/.ssh/rsyncKey"

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

for file in "${REMOTE_FILES[@]}"; do
    if [[ -f "$file" ]]; then
        rsync -avqz -e "ssh -i $KEY" --protect-args "$REMOTE" "$file"
    else
        echo "ERROR: Missing file ($file)"
        exit 1
    fi
done
