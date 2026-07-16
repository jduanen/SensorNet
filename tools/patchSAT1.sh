#!/bin/bash
#
# This script patches the FutureProofHomes Satellite1 Home Assistant Voice
#  Assistant to be compatible with SensorNet.
#
# N.B. Must use go-yq v4 ("mikefarah/yq"). Don't use the one installed by apt. 'sudo snap install yq'

# ~/Code/SensorNet/voiceAssistants/Satellite1
# ~/Code2/FutureProofHomes/Satellite1-ESPHome

# Diffs needed from upstream FutureProofHomes/Satellite1-ESPHome:
#
# esphome:
#   name: ${company_name}.${project_name}   (underscore → dot)
#
#   components: (change 8-space indent to 6-space)
#     - audio
#     - fusb302b
#     ...
#
# logger:
#   deassert_rts_dtr: true
#   hardware_uart: USB_SERIAL_JTAG  (remove extra space before colon)
#   level: debug                    (was ${log_level})
#
# packages:
#   led_ring: !include common/led_ring.yaml  (remove trailing space)
#   ## OPTIONAL COMPONENTS                   (remove trailing space)


#### FIXME
echo "ERROR: must implement patchSAT1"
exit 1
