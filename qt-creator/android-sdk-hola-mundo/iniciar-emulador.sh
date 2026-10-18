#!/usr/bin/env bash
set -euo pipefail
emulator="$HOME/Android/Sdk/emulator/emulator"
adb="$HOME/Android/Sdk/platform-tools/adb"
case "${1:-}" in
  --help|-h)
    printf '%s\n' 'Ubuntu físico: bash iniciar-emulador.sh' 'Alternativa lenta por software: bash iniciar-emulador.sh --software'
    exit 0 ;;
  --software)
    if "$adb" devices | awk '$1 == "emulator-5554" {found=1} END {exit !found}'; then
      printf '%s\n' 'Ya existe un emulador en emulator-5554. Ciérralo antes de iniciar otro.' >&2
      exit 2
    fi
    printf '%s\n' 'CPU por software, sin KVM. El arranque puede tardar varios minutos.'
    "$emulator" -avd Qt_Hola_API35 -port 5554 -accel off -gpu software -no-snapshot -no-boot-anim -memory 2048 -cores 2 &
    emulator_pid=$!
    trap 'kill "$emulator_pid" 2>/dev/null || true' EXIT
    timeout 600 "$adb" -s emulator-5554 wait-for-device
    "$adb" -s emulator-5554 root
    timeout 120 "$adb" -s emulator-5554 wait-for-device
    # Imagen AOSP de depuración del episodio: ampliar esperas de Android,
    # no las del host. El valor solo pertenece a esta sesión del emulador.
    "$adb" -s emulator-5554 shell setprop ro.hw_timeout_multiplier 10
    multiplier=$("$adb" -s emulator-5554 shell getprop ro.hw_timeout_multiplier | tr -d '\r')
    if [[ "$multiplier" != 10 ]]; then
      printf '%s\n' 'Esta imagen no permite ajustar las esperas. Usa el emulador acelerado sobre Ubuntu físico.' >&2
      exit 1
    fi
    "$adb" -s emulator-5554 shell 'stop; start'
    printf '%s\n' 'Espera a la pantalla de inicio de Android. Mantén esta terminal abierta.'
    wait "$emulator_pid"
    trap - EXIT
    exit 0 ;;
  '') ;;
  *) printf '%s\n' 'Opción desconocida. Consulta bash iniciar-emulador.sh --help' >&2; exit 2 ;;
esac
if systemd-detect-virt --vm --quiet; then
  printf '%s\n' 'No iniciamos KVM anidado dentro de una VM.' 'Usa Ubuntu físico para aceleración o consulta --software (alternativa experimental y lenta).' >&2
  exit 2
fi
"$emulator" -accel-check
exec "$emulator" -avd Qt_Hola_API35 -gpu software -no-snapshot -no-boot-anim -memory 2048 -cores 2
