#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build/device \
	-DCMAKE_TOOLCHAIN_FILE="$PLAYDATE_SDK_PATH/C_API/buildsupport/arm.cmake" \
	-DCMAKE_BUILD_TYPE=Release
cmake --build build/device
