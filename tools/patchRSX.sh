#!/bin/bash
#
# This script patches the ReSpeaker VXF3800 Home Assistant Voice Assistant to
#  be compatible with SensorNet.
#
# Changes:
#  - esphome:
#    * edit:
#      - name: ${device_name}
#      - friendly_name: ${friendly_name}
#    * add:
#      - comment: ${comment}
#  - logger:
#    * edit:
#      - level: ${log_level}
#  - sensor:
#    * add:
#      - - platform: wifi_signal
#          id: wifi_rssi
#          name: ${friendly_name} WiFi Signal"
#
# N.B. Must use go-yq v4 ("mikefarah/yq"). Don't use the one installed by apt. 'sudo snap install yq'

scriptDir="$(cd "$(dirname "$0")" && pwd)"
source "$scriptDir/commonPatch.sh"

SOURCE_FILE="${HOME}/Code2/Respeaker-XVF3800-ESPHome-integration/config/respeaker-xvf-satellite-example.yaml"

DST_FILE="${HOME}/Code/SensorNet/voiceAssistants/ReSpeakerVXF3800Satellite/packages/respeaker-xvf3800.yaml"

checkYQ

# pre-process YAML
YAML_FILE=$(mktemp --suffix=.yaml src-XXXX)
if ! cat $SOURCE_FILE | sed -E "s@(\x21lambda)[[:space:]]'(.+)'@\1 \2@" | sed -E "s/(\x21lambda[[:space:]].*$)/\'\1\'/"  > $YAML_FILE; then
    echo "ERROR: yaml file preprocessing failed"
    exit 1
fi
if [[ "$DEBUG" == true ]]; then
    diff $SOURCE_FILE $YAML_FILE
else
    rm -f $YAML_FILE
fi

convertToJson $YAML_FILE $SRC_FILE

# pre-process JSON

# edit JSON
updateJson "$SRC_FILE" "$TMP_FILE" '(.esphome.name = "${device_name}") | 
    (.esphome.friendly_name = "${friendly_name}") |
    (.esphome.comment = "${comment}") |
    (.wifi.ssid = "!secret wifi_ssid") |
    (.wifi.password = "!secret wifi_password") |
    (.logger.level = "${log_level}") |
    (.sensor += [{"platform": "wifi_signal", "id": "wifi_rssi", "name": "${friendly_name} WiFi Signal"}])'
if [[ "$DEBUG" == true ]]; then
    diff $SRC_FILE $TMP_FILE
fi

# convert to YAML and post-process it
if ! yq -P -o=yaml '.' $TMP_FILE | sed -E "s/'(\x21.* [^']*)'/\1/g" > $DST_FILE; then
    echo "ERROR: failed to convert file back to YAML"
    exit 1
fi

# cleanup
