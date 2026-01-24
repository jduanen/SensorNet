#!/bin/bash
#
# Script to validate that all of the config files pass syntax checks
#
# Have to be in the ESPHOME venv
#  workon ESPHOME
# Also needs to have symlink to secrets.yaml

source "${dirname "$0"}/common.sh"

LOG_FILE="/tmp/sensornet.log"

SUCCEEDED=
FAILED=

date > $LOG_FILE
echo "----------------" >> $LOG_FILE

for file in "${SOURCE_FILES[@]}"; do
    if [[ -f "$file" ]]; then
        if [[ -n "$VERBOSE" ]]; then
            echo -n "    Validating: $file ... "
        fi
        esphome -q config $file &>> $LOG_FILE
        if [[ $? -ne 0 ]]; then
            if [[ -n "$VERBOSE" ]]; then
                echo "Failed."
            fi
            if [[ -z "$FAILED" ]]; then
                FAILED=$file
            else
                FAILED="$FAILED $file"
            fi
            echo "^^^^^^^^ $file ^^^^^^^^" >> $LOG_FILE
        else
            if [[ -n "$VERBOSE" ]]; then
                echo "Succeeded."
            fi
            if [[ -z "$SUCCEEDED" ]]; then
                SUCCEEDED=$file
            else
                SUCCEEDED="$SUCCEEDED $file"
            fi
        fi
    else
        echo "ERROR: missing file ($file)"
        exit 1
    fi
done

echo ""
if [[ -n $SUCCEEDED ]]; then
    echo "Succeeded: "
    for s in $SUCCEEDED; do
        echo "    "$s
    done
    echo ""
fi
if [[ -n $FAILED ]]; then
    echo "Failed: "
    for f in $FAILED; do
        echo "    "$f
    done
    echo "See logs in: $LOG_FILE"
fi
