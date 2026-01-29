#!/bin/bash
#
# Script to update the HA voice assistants
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

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

baseDir="/home/jdn/Code2"

# Home Assistant Voice Assistant Preview Edition (PE)
patchHAVAPE () {
    if ! ${SCRIPT_DIR}/patchHAVAPE.sh; then
        echo "ERROR: Failed to patch HAVA PE"
        exit 1
    fi
}
d="home-assistant-voice-pe"
dirs+=(${d})
handlers[${d}]="patchHAVAPE"

# ReSpeakerV2
#### TODO

# ReSpeakerXVF3800
patchRSX () {
    if ! ${SCRIPT_DIR}/patchRSX.sh; then
        echo "ERROR: Failed to patch ReSpeaker-XVF3800"
        exit 1
    fi
}
d="Respeaker-XVF3800-ESPHome-integration"
dirs+=(${d})
handlers[${d}]="patchRSX"

# FutureProofHomes Satellite1
patchSAT1 () {
    echo "TBD: SAT1"
}
d="FutureProofHomes/satellite1-esphome"
dirs+=(${d})
handlers[${d}]="patchSAT1"

# Waveshare Satellite
patchWSS () {
    if ! ${SCRIPT_DIR}/patchWSS.sh; then
        echo "ERROR: Failed to patch Waveshare S2 Satellite"
        exit 1
    fi
}
d="waveshare-s2-audio_esphome_voice"
dirs+=(${d})
handlers["${d}"]="patchWSS"

# loop through dirs and check if updates are required
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
