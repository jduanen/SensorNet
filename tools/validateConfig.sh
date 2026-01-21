#!/bin/bash
#
# Script to validate that all of the config files pass syntax checks
#
# Have to be in the ESPHOME venv
#  workon ESPHOME

SOURCE_DIR="${HOME}/Code/SensorNet/configs/esphome"

SUCCEEDED=
FAILED=

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

for file in "${LOCAL_FILES[@]}"; do
  FILE_PATH="$SOURCE_DIR/$file"
  esphome -q config $FILE_PATH
  if [ $? -ne 0 ]; then
    if [ -z "$FAILED" ]; then
      FAILED=$FILE_PATH
    else
      FAILED=$FAILED" "$FILE_PATH
    fi
  else
    if [ -z "$SUCCEEDED" ]; then
      SUCCEEDED=$FILE_PATH
    else
      SUCCEEDED=$SUCCEEDED" "$FILE_PATH
    fi
  fi
done

echo ""
echo "Succeeded: "
for s in $SUCCEEDED; do
  echo "    "$s
done
echo ""
echo "Failed: "
for f in $FAILED; do
  echo "    "$f
done
