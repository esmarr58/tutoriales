#!/usr/bin/env bash
set -euo pipefail
# Ubuntu 24.04 x86_64, Qt 6.11/6.12 y Qt Creator 20.
# Ejecutar como usuario normal. sudo se usa únicamente para paquetes del sistema.
if (( EUID == 0 )); then
  echo 'Ejecuta bash preparar-sdk.sh como usuario normal, sin sudo.' >&2
  exit 1
fi
if [[ $(uname -m) != x86_64 ]]; then
  echo 'Este tutorial requiere Ubuntu x86_64.' >&2
  exit 1
fi
if pgrep -u "$UID" -x qtcreator >/dev/null; then
  echo 'Cierra Qt Creator antes de reemplazar las herramientas Android y vuelve a ejecutar este script.' >&2
  exit 1
fi

export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export PATH="$JAVA_HOME/bin:$PATH"
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_SDK_ROOT="$ANDROID_HOME"
tools_root="$ANDROID_SDK_ROOT/cmdline-tools"
material="$HOME/Downloads/qt-android-material"
tools_version=12.0
tools_sha256=2d2d50857e4eb553af5a6dc3ad507a17adf43d115264b1afc116f95c92e5e258
manager="$tools_root/latest/bin/sdkmanager"

sudo apt-get update
sudo apt-get install openjdk-21-jdk unzip curl
mkdir -p "$tools_root" "$material"

# Creator 20 utiliza latest/bin/sdkmanager, aunque exista una carpeta 12.0.
# No instalar cmdline-tools;latest: hoy descarga un CLI con otro formato.
unpack=''
trap 'if [[ -n "$unpack" ]]; then rm -rf -- "$unpack"; fi' EXIT
if [[ -L "$tools_root/latest" || ! -x "$manager" || ! -f "$tools_root/latest/source.properties" ]] || ! grep -Eq '^Pkg.Revision[[:space:]]*=[[:space:]]*12\.0[[:space:]]*$' "$tools_root/latest/source.properties"; then
  archive="$material/commandlinetools-linux-11076708.zip"
  if ! printf '%s  %s\n' "$tools_sha256" "$archive" | sha256sum -c - >/dev/null 2>&1; then
    curl -fL --retry 3 -o "$archive" https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
  fi
  printf '%s  %s\n' "$tools_sha256" "$archive" | sha256sum -c -
  unpack=$(mktemp -d "$tools_root/.preparar-sdk.XXXXXX")
  unzip -q "$archive" -d "$unpack"
  test -x "$unpack/cmdline-tools/bin/sdkmanager"
fi

# Retirar versiones anteriores del SDK, conservando un respaldo FUERA del SDK
# para evitar que sdkmanager las detecte como paquetes duplicados.
backup=''
retirar_herramientas() {
  if [[ -z "$backup" ]]; then
    mkdir -p "$material/respaldos"
    backup=$(mktemp -d "$material/respaldos/cmdline-tools-$(date +%Y%m%d-%H%M%S).XXXXXX")
  fi
  echo "Retirando $1 del SDK; respaldo en $backup/$(basename "$1")"
  mv -- "$1" "$backup/"
}
if [[ -n "$unpack" ]]; then
  if [[ -e "$tools_root/latest" || -L "$tools_root/latest" ]]; then
    retirar_herramientas "$tools_root/latest"
  fi
  mv -- "$unpack/cmdline-tools" "$tools_root/latest"
  rmdir "$unpack"
  unpack=''
fi
for previous in "$tools_root"/*; do
  [[ "$previous" == "$tools_root/latest" ]] && continue
  if [[ -f "$previous/source.properties" || -x "$previous/bin/sdkmanager" ]]; then
    retirar_herramientas "$previous"
  fi
done

# El ZIP manual no trae package.xml. Registrar la versión REAL bajo la ruta
# latest permite a Creator reconocer su paquete esencial sin descargar 23.0.
# Esto no acepta licencias: --licenses se ejecuta de forma interactiva después.
sed -i 's/^Pkg.Path=.*/Pkg.Path=cmdline-tools;latest/' "$tools_root/latest/source.properties"
cat > "$tools_root/latest/package.xml" <<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<ns2:repository xmlns:ns2="http://schemas.android.com/repository/android/common/02"
 xmlns:ns3="http://schemas.android.com/repository/android/generic/02"
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
 <localPackage path="cmdline-tools;latest" obsolete="false">
  <type-details xsi:type="ns3:genericDetailsType"/>
  <revision><major>12</major><minor>0</minor></revision>
  <display-name>Android SDK Command-line Tools (12.0 fijada para Qt Creator 20)</display-name>
 </localPackage>
</ns2:repository>
XML

echo "Comprobando command-line tools $tools_version y Java 21..."
"$JAVA_HOME/bin/java" -version
"$manager" --sdk_root="$ANDROID_SDK_ROOT" --version
echo 'Lee las licencias. Responde y únicamente si las aceptas.'
"$manager" --sdk_root="$ANDROID_SDK_ROOT" --licenses

# Consultar los paquetes antes de instalar; conservar otras API/NDK de proyectos
# existentes. Qt 6.11 y 6.12 necesitan los mismos componentes de esta lista.
installed=$("$manager" --sdk_root="$ANDROID_SDK_ROOT" --list_installed)
packages=(
  'platform-tools' 'platforms;android-36' 'build-tools;36.0.0'
  'ndk;27.2.12479018' 'emulator' 'system-images;android-35;default;x86_64'
)
missing=()
for package in "${packages[@]}"; do
  if awk -F '|' -v package="$package" '
    { path=$1; gsub(/^[[:space:]]+|[[:space:]]+$/, "", path); if (path == package) found=1 }
    END { exit !found }
  ' <<< "$installed"; then
    echo "Ya instalado: $package"
  else
    missing+=("$package")
  fi
done
if (( ${#missing[@]} )); then
  "$manager" --sdk_root="$ANDROID_SDK_ROOT" "${missing[@]}"
else
  echo 'Todos los componentes requeridos ya están instalados.'
fi
"$manager" --sdk_root="$ANDROID_SDK_ROOT" --list_installed
printf '\n%s\n' \
  'SDK preparado para Qt 6.11 y 6.12. En Qt Creator: Preferences > SDKs > Android:' \
  "JDK location: $JAVA_HOME" \
  "Android SDK location: $ANDROID_SDK_ROOT" \
  'Selecciona Java 21 aunque Creator conserve una ruta anterior a Java 17.' \
  'No actualices Android SDK Command-line Tools desde Creator: este tutorial fija 12.0.'
