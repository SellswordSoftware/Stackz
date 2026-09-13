#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build/simulator -DCMAKE_BUILD_TYPE=Debug
cmake --build build/simulator

if [[ ! -d stackz.pdx ]]; then
	"$PLAYDATE_SDK_PATH/bin/pdc" -sdkpath "$PLAYDATE_SDK_PATH" Source "$PWD/stackz.pdx"
fi
