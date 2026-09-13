#!/usr/bin/env bash
set -euo pipefail
exec "$PLAYDATE_SDK_PATH/bin/PlaydateSimulator" "$PWD/stackz.pdx"
