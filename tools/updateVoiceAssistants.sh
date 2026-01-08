#!/bin/bash
#
# Script to update the HA voice assistants
#

checkRepoOLD() {
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

inGitRepo() {
    if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        echo "Not inside a git repository."
        exit 1
    fi
}

checkRepo() {
    local gitDir="$1"
    (
        cd "${gitDir}"
        if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
            echo "Not inside a git repository."
            exit 1
        fi
        git fetch --quiet
        read behind ahead < <(git rev-list --left-right --count @{u}...HEAD)
        if ! git diff --quiet || ! git diff --cached --quiet; then
            dirty=1
        else
            dirty=0
        fi

        echo "  Status relative to upstream:"
        echo "    Behind: $behind commit(s)"
        echo "    Ahead : $ahead commit(s)"
        echo "    Dirty : $dirty"

        # if behind and clean, pull
        if (( behind > 0 )); then
            if (( dirty == 1 )); then
                echo "  There are uncommitted changes; refusing to pull."
                echo "  Commit/stash your changes first."
            exit 1
            fi
            echo "  Pulling latest changes..."
            git pull --ff-only
            echo "  Repository updated."
        else
            echo "  Already up to date with upstream."
        fi

        # warning if ahead
        if (( ahead > 0 )); then
            echo "  Note: You are ahead of upstream by $ahead commit(s). Consider git push."
        fi
    )
    return $dirty
}

declare -A handlers
dirs=()

baseDir="/home/jdn/Code2/"

# Home Assistant Voice Assistant Preview Edition (PE)
patchHAVAPE () {
    echo "TBD: HAVAPE"
}
d="home-assistant-voice-pe"
dirs+=(${d})
handlers[${d}]="patchHAVAPE"

# ReSpeakerXVF3800
patchRSX () {
    echo "TBD: RSX"
}
d="Respeaker-XVF3800-ESPHome-integration"
dirs+=(${d})
handlers[${d}]="patchRSX"

# Waveshare Satellite
patchWSS () {
    echo "TBD: WSS"
}
d="waveshare-s2-audio_esphome_voice"
dirs+=(${d})
handlers["${d}"]="patchWSS"


for dir in "${dirs[@]}"; do
    path="${baseDir}/${dir}"
    echo "Checking: ${path##*/} @ ${path}"
    checkRepo "${path}"; r=$?
    if [ $r == 1 ]; then
        echo "  Was updated, needs to be patched"
        echo "    patching with: ${handlers[$dir]}"
        ${handlers[$dir]}
    fi
    echo ""
done
