#!/usr/bin/env bash
set -euo pipefail
emulator="$HOME/Android/Sdk/emulator/emulator"
"$emulator" -accel-check
exec "$emulator" -avd Qt_Hola_API35 -gpu software -no-snapshot -no-boot-anim -memory 2048 -cores 2
