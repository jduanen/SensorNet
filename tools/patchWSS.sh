#!/bin/bash
#
# This script patches the Waveshare S3 Satellite Home Assistant Voice Assistant
#  device to be compatible with SensorNet.
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
#  - api:
#    * edit:
#      - key: !secret api_encryption_key
#  - sensor:
#    * add:
#      - - platform: wifi_signal
#          id: wifi_rssi
#          name: ${friendly_name} WiFi Signal"

DEBUG=${DEBUG:-false}

SOURCE_FILE="${HOME}/Code2/waveshare-s2-audio_esphome_voice/waveshare-s3-audio.yaml"

SRC_FILE="/tmp/src.yaml"
TMP_FILE="/tmp/tmp.yaml"
DST_FILE="${HOME}/Code/SensorNet/voiceAssistants/WaveshareSatellite/packages/waveshare-audio-esp32-s3.yaml"

updateYaml() {
    local opts="$1"
    shift
    local file="$1"
    shift

    if ! yq $opts "$@" "$file"; then
        echo "ERROR: yq failed on file '$file'"
        exit 1
    fi
}

#### TMP TMP TMP just fixing a typo in the source file
sed -e 's/[[:space:]]*$//' -e '1194s/^  //' $SOURCE_FILE > $TMP_FILE
## cp $SOURCE_FILE $TMP_FILE
updateYaml -yr $TMP_FILE '.' > $SRC_FILE

if [[ "$DEBUG" == "true" ]]; then
    ls -l $SRC_FILE $TMP_FILE
fi

#### FIXME combine operators for efficiency
#yq -yri '(.esphome.name = "${device_name}") | (.esphome.name_add_mac_suffix = false) , (.wifi.ssid = "!secret wifi_ssid") | (.wifi.password = "!secret wifi_password")' $TMP_FILE
updateYaml -yri $TMP_FILE '(.esphome.name = "${device_name}")'
updateYaml -yri $TMP_FILE '(.esphome.friendly_name = "${friendly_name}")'
updateYaml -yri $TMP_FILE '(.esphome.comment = "${comment}")'
updateYaml -yri $TMP_FILE '(.logger.level = "${log_level}")'
updateYaml -yri $TMP_FILE '(.api.key = "!secret api_encryption_key")'
updateYaml -yri $TMP_FILE '(.sensor += [{"platform": "wifi_signal", "id": "wifi_rssi", "name": "${friendly_name} WiFi Signal"}])'

cp $TMP_FILE $DST_FILE

if [[ "$DEBUG" == "true" ]]; then
    echo ""
    ls -l $SRC_FILE $TMP_FILE
    tkdiff $SRC_FILE $TMP_FILE
else
    rm $SRC_FILE $TMP_FILE
fi
