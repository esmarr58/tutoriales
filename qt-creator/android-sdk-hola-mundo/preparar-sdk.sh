#!/usr/bin/env bash
set -euo pipefail
# Ejecutar como usuario normal. sudo se usa únicamente para paquetes del sistema.
sudo apt-get update
sudo apt-get install openjdk-21-jdk unzip curl
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
mkdir -p "$ANDROID_SDK_ROOT/cmdline-tools" "$HOME/Downloads/qt-android-material"
if [ ! -x "$ANDROID_SDK_ROOT/cmdline-tools/12.0/bin/sdkmanager" ]; then
  cd "$HOME/Downloads/qt-android-material"
  curl -fL --retry 3 -o commandlinetools.zip https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
  printf '%s\n' '2d2d50857e4eb553af5a6dc3ad507a17adf43d115264b1afc116f95c92e5e258  commandlinetools.zip' | sha256sum -c -
  # No sustituir una instalación parcial existente sin revisión.
  if [ -e "$ANDROID_SDK_ROOT/cmdline-tools/12.0" ]; then
    echo 'La carpeta 12.0 existe pero no contiene sdkmanager. Revísala antes de continuar.' >&2; exit 1
  fi
  unpack=$(mktemp -d)
  unzip -q commandlinetools.zip -d "$unpack"
  mv "$unpack/cmdline-tools" "$ANDROID_SDK_ROOT/cmdline-tools/12.0"
  rmdir "$unpack"
fi
manager="$ANDROID_SDK_ROOT/cmdline-tools/12.0/bin/sdkmanager"
echo 'Lee las licencias. Responde y únicamente si las aceptas.'
"$manager" --sdk_root="$ANDROID_SDK_ROOT" --licenses
"$manager" --sdk_root="$ANDROID_SDK_ROOT" \
  'cmdline-tools;latest' 'platform-tools' \
  'platforms;android-36' 'build-tools;36.0.0' \
  'platforms;android-37.0' 'build-tools;37.0.0' \
  'ndk;27.2.12479018' 'emulator' 'system-images;android-35;default;x86_64'
java -version
"$manager" --sdk_root="$ANDROID_SDK_ROOT" --list_installed
echo 'SDK instalado. Continúa con Preferences > SDKs > Android en Qt Creator.'
