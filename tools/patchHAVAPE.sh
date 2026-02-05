#!/bin/bash
#
# This script patches the Home Assistant Voice Assistant PE device to be
#  compatible with SensorNet.
#
# Changes:
#  - esphome:
#    * edit:
#      - name: ${device_name}
#      - name_add_mac_suffix: false
#    * add:
#      - friendly_name: ${friendly_name}
#      - comment: ${comment}
#  - wifi:
#    * add:
#      - ssid: !secret wifi_ssid
#      - password: !secret wifi_password
#  - logger:
#    * edit:
#      - level: ${log_level}
#  - api:
#    * edit:
#      - encryption:
#      * add:
#        - key: !secret api_encryption_key
#  - sensor:
#    * add:
#      - - platform: wifi_signal
#          id: wifi_rssi
#          name: ${friendly_name} WiFi Signal"
#
# N.B. Must use go-yq v4 ("mikefarah/yq"). Don't use the one installed by apt. 'sudo snap install yq'

#### FIXME  remove single quotes from all that use !secret: e.g., key: '!secret api_encryption_key', and wifi/passwd
#### FIXME  add '!lambda ' to all like this: 'brightness: return max( id(led_ring).current_values.get_brightness() , 0.2f );'


scriptDir="$(cd "$(dirname "$0")" && pwd)"
source "$scriptDir/commonPatch.sh"

SOURCE_FILE="${HOME}/Code2/home-assistant-voice-pe/home-assistant-voice.yaml"

DST_FILE="${HOME}/Code/SensorNet/voiceAssistants/HomeAssistantVoicePE/packages/home-assistant-voice.yaml"

checkYQ

convertToJson $SOURCE_FILE $SRC_FILE

updateJson "$SRC_FILE" "$TMP_FILE" '(.esphome.name = "${device_name}") | 
    (.esphome.name_add_mac_suffix = false) |
    (.esphome.friendly_name = "${friendly_name}") |
    (.esphome.comment = "${comment}") |
    (.wifi.ssid = "!secret wifi_ssid") |
    (.wifi.password = "!secret wifi_password") |
    (.logger.level = "${log_level}") |
    (.api.encryption.key = "!secret api_encryption_key") |
    (.sensor += [{"platform": "wifi_signal", "id": "wifi_rssi", "name": "${friendly_name} WiFi Signal"}])'

if ! yq -P -o=yaml '.' $TMP_FILE > $DST_FILE; then
    echo "ERROR: failed to convert file back to YAML"
    exit 1
fi
