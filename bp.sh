#!/usr/bin/env bash
set -euo pipefail

pdxinfo_file="Source/pdxinfo"
if [[ ! -f "$pdxinfo_file" ]]; then
	echo "Missing $pdxinfo_file" >&2
	exit 1
fi

current_build=$(awk -F= '$1 == "buildNumber" { print $2; found = 1; exit } END { if (!found) exit 1 }' "$pdxinfo_file")
if [[ ! "$current_build" =~ ^[0-9]+$ ]]; then
	echo "Invalid buildNumber in $pdxinfo_file: $current_build" >&2
	exit 1
fi

next_build=$((10#$current_build + 1))
temporary_pdxinfo=$(mktemp "${pdxinfo_file}.XXXXXX")
trap 'rm -f "$temporary_pdxinfo"' EXIT
awk -v build="$next_build" '
	$0 ~ /^buildNumber=/ { print "buildNumber=" build; found = 1; next }
	{ print }
	END { if (!found) exit 1 }
' "$pdxinfo_file" > "$temporary_pdxinfo"
mv "$temporary_pdxinfo" "$pdxinfo_file"
trap - EXIT
echo "Incremented buildNumber: $current_build -> $next_build"

cmake -S . -B build/device \
	-DCMAKE_TOOLCHAIN_FILE="$PLAYDATE_SDK_PATH/C_API/buildsupport/arm.cmake" \
	-DCMAKE_BUILD_TYPE=Release
cmake --build build/device

if [[ ! -d stackz_DEVICE.pdx ]]; then
	"$PLAYDATE_SDK_PATH/bin/pdc" -sdkpath "$PLAYDATE_SDK_PATH" Source "$PWD/stackz_DEVICE.pdx"
fi
