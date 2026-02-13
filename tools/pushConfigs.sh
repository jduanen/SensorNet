#!/bin/bash
#
# Script to push ESPHome config files from HA Server to remote server
#
# Create key on HA server first:
#  ssh-keygen -t ed25519 -f ~/.ssh/rsyncKey -C "rsync automation key"
#  chmod 600 ~/.ssh/rsyncKey
#  chmod 644 ~/.ssh/rsyncKey.pub
#  ssh-copy-id -i ~/.ssh/rsyncKey.pub jdn@gpuServer1.local
# Test with:
#  ssh -i /root/.ssh/rsyncKey jdn@gpuServer1.local

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

SCRIPT_DIR="$(dirname "$0")"
source "${SCRIPT_DIR}/common.sh"

LOCAL_DIR="/root/config/esphome"

KEY="~/.ssh/rsyncKey"

for remoteFile in "${SOURCE_FILES[@]}"; do
    localFile="$LOCAL_DIR/${remoteFile##*/}"
    if [[ "$DEBUG" == true ]]; then
        echo "$localFile -> $remoteFile"
    fi
    if [[ -f "$localFile" ]]; then
        rsync -avqz -e "ssh -i $KEY" --protect-args "$localFile" "$REMOTE_USER_HOST:$remoteFile"
    else
        if [[ "$DEBUG" == true ]]; then
            echo "WARNING: Missing local file ($localFile)"
        fi
    fi
done
