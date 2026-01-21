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

REMOTE="jdn@gpuServer1.local:Code/SensorNet/config/esphome/"

SOURCE_DIR="/root/config/esphome"

LOCAL_FILES=(
    "feeder-door.yaml"
    "air-quality-pms-0.yaml"
    "air-quality-pms-1.yaml"
    "air-quality-pms-2.yaml"
    "air-quality-sps-0.yaml"
    "env-sensors.yaml"
    "kittycam.yaml"
    "kittycam2.yaml"
    "led-sign.yaml"
    "radiation-0.yaml"
    "radiation-1.yaml"
    "smart-plug-ems01-0.yaml"
    "tempsense0.yaml"
    "tempsense1.yaml"
    "water-temperature.yaml"
    "waterheater-leak-detector.yaml"
    "home-assistant-voice-0.yaml"
    "respeaker-xvf3800-0.yaml"
    "satellite1-cfec40.yaml"
    "waveshare-audio-0.yaml"
    "waveshare-audio-1.yaml"
)

KEY="~/.ssh/rsyncKey"

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

for file in "${LOCAL_FILES[@]}"; do
  FILE_PATH="$SOURCE_DIR/$file"
  if [[ -f "$FILE_PATH" ]]; then
    rsync -avqz -e "ssh -i $KEY" --protect-args "$FILE_PATH" "$REMOTE"
  else
    echo "ERROR: Skipping missing file ($FILE_PATH)"
  fi
done
