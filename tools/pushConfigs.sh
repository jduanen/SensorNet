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

source "${dirname "$0"}/common.sh"

LOCAL_DIR="/root/config/esphome"

KEY="~/.ssh/rsyncKey"

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

for remoteFile in "${REMOTE_FILES[@]}"; do
    localFile="$LOCAL_DIR/${remoteFile##*/}"
    rsync -avqz -e "ssh -i $KEY" --protect-args "$localFile" "$REMOTE_USER_HOST:$remoteFile"
done
