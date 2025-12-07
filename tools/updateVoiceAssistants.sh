#!/bin/bash
#
# Script to update the HA voice assistants
#

checkRepo() {
    local gitDir="$1"
    (
        cd "${gitDir}" && \
        gitStatus=$(git status -uno) && \
        if echo "$gitStatus" | grep -q "Your branch is up to date"; then
            echo "up to date"
            upToDate=0
        else
            echo "needs an update"
            upToDate=1
        fi
    )
    return $upToDate
}

# Home Assistant Voice Assistant Preview Edition (PE)
d="/home/jdn/Code2/home-assistant-voice-pe"
r=$(checkRepo "$d")
if [ $? == 0 ]; then
    # up to date, don't have to do anything
    echo "${d##*/}: is up to date"
else
    echo "${d##*/}: needs to be updated"
    #### TODO apply patch
fi

# ReSpeakerXVF3800
d="/home/jdn/Code2/Respeaker-XVF3800-ESPHome-integration"
r=$(checkRepo "$d")
if [ $? == 0 ]; then
    # up to date, don't have to do anything
    echo "${d##*/}: is up to date"
else
    echo "${d##*/}: needs to be updated"
    #### TODO apply patch
fi

# Waveshare Satellite
d="/home/jdn/Code2/waveshare-s2-audio_esphome_voice"
r=$(checkRepo "$d")
if [ $? == 0 ]; then
    # up to date, don't have to do anything
    echo "${d##*/}: is up to date"
else
    echo "${d##*/}: needs to be updated"
    #### TODO apply patch
fi
