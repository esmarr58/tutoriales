#!/usr/bin/env bash
set -euo pipefail
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
manager="$ANDROID_SDK_ROOT/cmdline-tools/12.0/bin/avdmanager"
test -x "$manager" || { echo 'Primero ejecuta preparar-sdk.sh'; exit 1; }
if "$ANDROID_SDK_ROOT/emulator/emulator" -list-avds | grep -Fxq Qt_Hola_API35; then
  echo 'Qt_Hola_API35 ya existe; se conserva.'
else
  printf 'no\n' | "$manager" create avd -n Qt_Hola_API35 -k 'system-images;android-35;default;x86_64' -d pixel_5
fi
"$ANDROID_SDK_ROOT/emulator/emulator" -list-avds
