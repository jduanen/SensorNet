#!/usr/bin/env bash
set -euo pipefail

HA_HOST="homeassistant.local"
HA_USER="root"
HA_DEST="/config/www/led-sign/index.html"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST="$SCRIPT_DIR/dashboard/dist/index.html"

echo "Building..."
(cd "$SCRIPT_DIR/dashboard" && npm run build --silent)

echo "Deploying to $HA_USER@$HA_HOST:$HA_DEST"
ssh "$HA_USER@$HA_HOST" "mkdir -p $(dirname "$HA_DEST")"
scp "$DIST" "$HA_USER@$HA_HOST:$HA_DEST"

echo "Done. Hard-refresh the browser to pick up changes."
