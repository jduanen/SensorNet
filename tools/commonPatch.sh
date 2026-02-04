#
# Common Code for patching HA Voice Assistant YAML config files
#

DEBUG=${DEBUG:-false}

SRC_FILE=$(mktemp --suffix=.json src-XXXX)
TMP_FILE=$(mktemp --suffix=.json tmp-XXXX)

trap 'cleanup' EXIT

checkYQ() {
    if ! yq --version | egrep mikefarah > /dev/null; then
        echo "ERROR: wrong version of yq"
        exit 1
    fi
}

updateJson() {
    local inFile="$1"
    shift
    local outFile="$1"
    shift

    if ! jq "$@" "$inFile" >> "$outFile"; then
        echo "ERROR: jq failed on file '$inFile'"
        exit 1
    fi
}

convertToJson() {
    if ! yq -o=json '.' "$1" | jq '.' > "$2"; then
        echo "ERROR: failed to convert source file '$1' to JSON"
        exit 1
    fi
}

cleanup() {
    if [[ "$DEBUG" != true ]]; then
        rm -f "$SRC_FILE" "$TMP_FILE"
    fi
}
