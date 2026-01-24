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

source "${dirname "$0"}/common.sh"

LOCAL_DIR="/root/config/esphome"

KEY="~/.ssh/rsyncKey"

trap 'echo "Interrupted by Ctrl+C"; exit 130' INT

for remoteFile in "${REMOTE_FILES[@]}"; do
    localFile="${remoteFile##*/}"
    if [[ -f "$localFile" ]]; then
        rsync -avqz -e "ssh -i $KEY" --protect-args "$REMOTE_USER_HOST:$remoteFile" "$localFile"
    else
        echo "ERROR: Missing file ($localFile)"
        exit 1
    fi
done
