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

echo "FIXME: corrupts parts with lambdas"
exit 1

DEBUG=${DEBUG:-false}

SOURCE_FILE="${HOME}/Code2/Respeaker-XVF3800-ESPHome-integration/config/respeaker-xvf-satellite-example.yaml"

SRC_FILE="/tmp/src.yaml"
TMP_FILE="/tmp/tmp.yaml"
DST_FILE="${HOME}/Code/SensorNet/voiceAssistants/ReSpeakerVXF3800Satellite/packages/respeaker-xvf3800.yaml"

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

updateYaml -yr $SOURCE_FILE '.' > $SRC_FILE
cp $SRC_FILE $TMP_FILE
if [[ "$DEBUG" == "true" ]]; then
    ls -l $SRC_FILE $TMP_FILE
fi

#### FIXME combine operators for efficiency
#yq -yri '(.esphome.name = "${device_name}") | (.esphome.name_add_mac_suffix = false) , (.wifi.ssid = "!secret wifi_ssid") | (.wifi.password = "!secret wifi_password")' $TMP_FILE
updateYaml -yri $TMP_FILE '(.esphome.name = "${device_name}")'
updateYaml -yri $TMP_FILE '(.esphome.friendly_name = "${friendly_name}")'
updateYaml -yri $TMP_FILE '(.esphome.comment = "${comment}")'
updateYaml -yri $TMP_FILE '(.logger.level = "${log_level}")'
updateYaml -yri $TMP_FILE '(.sensor += [{"platform": "wifi_signal", "id": "wifi_rssi", "name": "${friendly_name} WiFi Signal"}])'

cp $TMP_FILE $DST_FILE

if [[ "$DEBUG" == "true" ]]; then
    echo ""
    ls -l $SRC_FILE $TMP_FILE
    tkdiff $SRC_FILE $TMP_FILE
else
    rm $SRC_FILE $TMP_FILE
fi
