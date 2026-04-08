#!/bin/bash
#
# Script to update the HA voice assistants
#
# Usage:
#   ./update.sh               # Normal mode: check repos and patch if updated
#   FORCE_UPDATE=1 ./update.sh  # Force patching regardless of repo status
#

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
FORCE_UPDATE="${FORCE_UPDATE:-0}"

# Return codes for checkRepo
readonly RC_UP_TO_DATE=0
readonly RC_UPDATED=1
readonly RC_ERROR=2

# Check a git repo for updates, pull if behind and clean.
# Returns:
#   RC_UP_TO_DATE (0) - no changes pulled
#   RC_UPDATED    (1) - successfully pulled new changes
#   RC_ERROR      (2) - error (not a repo, dirty + behind, pull failure)
checkRepo() {
    local gitDir="$1"

    if [[ ! -d "${gitDir}" ]]; then
        echo "  ERROR: Directory does not exist: ${gitDir}"
        return $RC_ERROR
    fi

    # Change into the repo directory (in the current shell, we'll cd back via pushd/popd)
    pushd "${gitDir}" > /dev/null || return $RC_ERROR

    if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        echo "  ERROR: Not inside a git repository."
        popd > /dev/null
        return $RC_ERROR
    fi

    if ! git fetch --quiet 2>/dev/null; then
        echo "  WARNING: git fetch failed (network issue?). Proceeding with local state."
    fi

    local behind ahead dirty=0
    read -r behind ahead < <(git rev-list --left-right --count @{u}...HEAD 2>/dev/null) || {
        echo "  ERROR: Could not determine upstream status. Is an upstream branch configured?"
        popd > /dev/null
        return $RC_ERROR
    }

    if ! git diff --quiet 2>/dev/null || ! git diff --cached --quiet 2>/dev/null; then
        dirty=1
    fi

    echo "  Status relative to upstream:"
    echo "    Behind: ${behind} commit(s)"
    echo "    Ahead : ${ahead} commit(s)"
    echo "    Dirty : ${dirty}"

    # Warning if ahead
    if (( ahead > 0 )); then
        echo "  WARNING: Ahead of upstream by ${ahead} commit(s). Consider 'git push'."
    fi

    local result=$RC_UP_TO_DATE

    if (( behind > 0 )); then
        if (( dirty == 1 )); then
            echo "  ERROR: Uncommitted changes present; refusing to pull."
            echo "  Commit or stash your changes first."
            popd > /dev/null
            return $RC_ERROR
        fi
        echo "  Pulling latest changes..."
        if git pull --ff-only; then
            echo "  Repository updated."
            result=$RC_UPDATED
        else
            echo "  ERROR: git pull --ff-only failed. Manual intervention required."
            popd > /dev/null
            return $RC_ERROR
        fi
    else
        echo "  Already up to date with upstream."
    fi

    popd > /dev/null
    return $result
}

# -------------------------------------------------------------------
# Define repositories and their associated patch scripts
# -------------------------------------------------------------------
declare -A handlers
dirs=()

baseDir="/home/jdn/Code2"

# Home Assistant Voice Assistant Preview Edition (PE)
d="home-assistant-voice-pe"
dirs+=("${d}")
handlers["${d}"]="patchHAVAPE.sh"

# ReSpeaker XVF3800
d="Respeaker-XVF3800-ESPHome-integration"
dirs+=("${d}")
handlers["${d}"]="patchRSX.sh"

# FutureProofHomes Satellite1
d="FutureProofHomes/satellite1-esphome"
dirs+=("${d}")
handlers["${d}"]="patchSAT1.sh"

# Waveshare Satellite
d="waveshare-s2-audio_esphome_voice"
dirs+=("${d}")
handlers["${d}"]="patchWSS.sh"

# -------------------------------------------------------------------
# Main loop
# -------------------------------------------------------------------
overall_exit=0

for dir in "${dirs[@]}"; do
    doUpdate=0
    path="${baseDir}/${dir}"
    displayName="${path##*/}"

    echo "============================================"
    echo "Project: ${displayName}"
    echo "Path:    ${path}"
    echo "============================================"

    if (( FORCE_UPDATE != 0 )); then
        echo "  FORCE_UPDATE is set — will patch regardless of repo state."
        doUpdate=1
    else
        # Temporarily disable errexit so we can capture the return code
        set +e
        checkRepo "${path}"
        doUpdate=$?
        set -e

        if (( doUpdate == RC_ERROR )); then
            echo "  Skipping ${displayName} due to errors."
            overall_exit=1
            echo ""
            continue
        fi
    fi

    if (( doUpdate == RC_UPDATED )) || (( FORCE_UPDATE != 0 )); then
        localHandler="${handlers[${dir}]}"
        handlerPath="${SCRIPT_DIR}/${localHandler}"

        if [[ ! -x "${handlerPath}" ]]; then
            echo "  ERROR: Patch script not found or not executable: ${handlerPath}"
            overall_exit=1
            echo ""
            continue
        fi

        echo "  Running patch: ${localHandler}"
        # Temporarily disable errexit for the patch script
        set +e
        "${handlerPath}"
        patch_rc=$?
        set -e

        if (( patch_rc == 0 )); then
            echo "  ✓ Patched ${displayName} successfully."
        else
            echo "  ✗ Failed to patch ${displayName} (exit code: ${patch_rc})."
            overall_exit=1
        fi
    else
        echo "  No patch needed."
    fi

    echo ""
done

if (( overall_exit != 0 )); then
    echo "WARNING: One or more projects had errors. Review output above."
fi

exit $overall_exit
