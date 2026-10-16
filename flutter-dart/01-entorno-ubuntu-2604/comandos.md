# Flutter y Dart 01 · Comandos del episodio

Objetivo: preparar Ubuntu 26.04 x64 para crear y ejecutar una aplicación Flutter de escritorio Linux con VS Code. Ensayo y grabación: Ubuntu 26.04.1, Flutter 3.47.7, Dart 3.13.5 y VS Code 1.142.0. Los comandos de instalación se ejecutan en **Ubuntu**, en una terminal. Pausa el video para realizarlos.

## 1. Dependencias

```bash
sudo apt-get update
sudo apt-get install -y \
  ca-certificates curl git unzip xz-utils zip libglu1-mesa \
  clang cmake ninja-build pkg-config \
  libgtk-3-dev libstdc++-12-dev
```

## 2. SDK de Flutter (incluye Dart)

El archivo de este ejemplo es **Linux x64**, para Intel/AMD de 64 bits. Comprueba la arquitectura con `uname -m`: en el video devuelve `x86_64`. Para otra arquitectura, consulta el [archivo oficial de Flutter](https://docs.flutter.dev/install/archive).

```bash
mkdir -p ~/Downloads
cd ~/Downloads
curl -fL 'https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_3.47.7-stable.tar.xz' \
  -o flutter_linux_3.47.7-stable.tar.xz
echo 'af7455f540df951184c7953fbe3cf884766e7541dacf8571f9f582db03dd6230  flutter_linux_3.47.7-stable.tar.xz' \
  | sha256sum --check
```

La última instrucción debe terminar en `OK`. La huella corresponde exactamente a este archivo, publicada en el [manifiesto oficial de versiones Linux](https://storage.googleapis.com/flutter_infra_release/releases/releases_linux.json). Extrae después de comprobarla, sin usar `sudo`:

```bash
mkdir -p ~/develop
tar -xf flutter_linux_3.47.7-stable.tar.xz -C ~/develop
```

El ejemplo parte de una carpeta `~/develop/flutter` que todavía no existe. La carpeta Downloads se muestra como Descargas en algunas interfaces.

## 3. Guardar PATH

Abre tu archivo de configuración:

```bash
nano ~/.bashrc
```

Añade **una vez**, al final y conservando el contenido anterior:

```bash
export PATH="$HOME/develop/flutter/bin:$PATH"
```

En nano: `Ctrl+O`, `Enter` para guardar; `Ctrl+X` para salir. Cierra esa terminal y abre otra. Comprueba:

```bash
flutter --version
dart --version
```

## 4. VS Code y extensiones

```bash
cd ~/Downloads
curl -fL 'https://update.code.visualstudio.com/latest/linux-deb-x64/stable' -o code.deb
sudo apt install ./code.deb
code --install-extension Dart-Code.flutter
```

La extensión Flutter es del publicador **Dart Code** y agrega también su extensión Dart. También puedes buscarla en la vista Extensiones de VS Code. El enlace de VS Code entrega la versión estable disponible al descargarlo. Puedes usar el editor para esta práctica sin iniciar sesión en una cuenta.

En VS Code, `Ctrl+Shift+P` → **Preferences: Open User Settings (JSON)**. Integra esta clave dentro del objeto existente, conservando tus otros ajustes y las comas del JSON:

```json
"dart.flutterSdkPath": "/home/TU_USUARIO/develop/flutter"
```

Cambia `TU_USUARIO` por tu usuario de Ubuntu. La ruta termina en `flutter`, sin `bin`; aquí no se expande `$HOME`. Los ajustes de tamaño de letra que se ven en la grabación solo mejoran la legibilidad.

## 5. Comprobar Linux

```bash
flutter config --enable-linux-desktop
flutter doctor -v
flutter devices
```

Busca **Linux toolchain** disponible y **Linux (desktop)** en dispositivos. En este episodio no se instalan Android SDK ni Chrome: sus avisos corresponden a los destinos Android y Web. En la VM apareció también un aviso sobre `eglinfo`; la app Linux compiló y abrió correctamente. Revisa los errores del destino que realmente vayas a usar.

## 6. Crear, comprobar y ejecutar

Desde una carpeta que todavía no contenga un proyecto llamado `hola_flutter`:

```bash
mkdir -p ~/proyectos
cd ~/proyectos
flutter create --platforms=linux hola_flutter
cd hola_flutter
code .
flutter analyze
flutter test
flutter run -d linux
```

Si VS Code pregunta por la confianza, habilita la carpeta de este proyecto que acabas de crear. Abre `lib/main.dart`; conserva el ejemplo inicial. El análisis debe terminar sin problemas y la prueba del contador debe pasar. Al abrir la app, comprueba el contador y pulsa más. Para salir de `flutter run`, vuelve a la terminal y pulsa `q`.

Reto del episodio: abrir una terminal nueva, comprobar las versiones y ejecutar otra vez desde la carpeta del proyecto. En una ejecución nueva, tres pulsaciones deben dejar el contador en 3.

Referencias: [Flutter para Linux](https://docs.flutter.dev/platform-integration/linux/setup), [VS Code en Linux](https://code.visualstudio.com/docs/setup/linux), [ajustes de Dart Code](https://dartcode.org/docs/settings/).
