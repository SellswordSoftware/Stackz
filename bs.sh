#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build/simulator -DCMAKE_BUILD_TYPE=Debug
cmake --build build/simulator
