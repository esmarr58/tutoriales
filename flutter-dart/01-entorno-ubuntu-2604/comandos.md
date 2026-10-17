# Flutter y Dart 01 · Instalación y ejecución paso a paso

Objetivo: preparar **Ubuntu 26.04 x64**, descargar el proyecto de este repositorio y ejecutar el contador como aplicación de escritorio Linux. El recorrido parte de Ubuntu sin Flutter instalado. Necesitas internet y una cuenta con permisos `sudo`.

Versiones del ensayo previo: Ubuntu 26.04.1, Flutter 3.47.7, Dart 3.13.5 y VS Code 1.142.0. La descarga de VS Code de abajo entrega la versión estable disponible al descargarla. No se instala un Dart independiente: viene incluido en Flutter.

Todos los comandos se ejecutan en una **terminal de Ubuntu**. Ábrela con `Ctrl+Alt+T`. Pega con `Ctrl+Shift+V` y ejecuta con `Enter`. No copies el indicador que aparece antes del comando. Espera a que vuelva antes de ejecutar el siguiente. Las barras `\` al final de una línea continúan el mismo comando: copia el bloque completo.

## 1. Arquitectura y paquetes

```bash
uname -m
```

Debe devolver `x86_64` para seguir estos pasos y descargar el archivo x64. Si devuelve otra arquitectura, el paquete de este recorrido no corresponde a tu equipo: consulta el [archivo oficial de Flutter](https://docs.flutter.dev/install/archive).

Ejecuta estos tres bloques en orden. Si `sudo` pide contraseña, escribe la de tu usuario y pulsa Enter; no aparecen caracteres mientras escribes.

```bash
sudo apt-get update
```

```bash
sudo apt-get install -y \
  ca-certificates curl git unzip xz-utils zip \
  libglu1-mesa nano xdg-user-dirs
```

```bash
sudo apt-get install -y \
  clang cmake ninja-build pkg-config \
  libgtk-3-dev libstdc++-12-dev
```

## 2. Descargar y extraer Flutter

Abre el [archivo oficial](https://docs.flutter.dev/install/archive), selecciona **Linux**, versión **3.47.7**, canal **stable**, arquitectura **x64**. Descarga `flutter_linux_3.47.7-stable.tar.xz` en la carpeta Descargas y espera a que termine.

En la terminal, entra a la carpeta de descargas configurada por Ubuntu. Este comando funciona tanto si se llama `Downloads` como si se llama `Descargas`:

```bash
cd "$(xdg-user-dir DOWNLOAD)"
pwd
sha256sum flutter_linux_3.47.7-stable.tar.xz
```

Compara **todos los caracteres** del resultado con esta huella SHA-256, publicada en el [manifiesto oficial de versiones Linux](https://storage.googleapis.com/flutter_infra_release/releases/releases_linux.json):

```text
af7455f540df951184c7953fbe3cf884766e7541dacf8571f9f582db03dd6230
```

Si no coincide, descarga de nuevo antes de extraer. Si coincide, ejecuta:

```bash
mkdir -p "$HOME/develop"
tar -xf flutter_linux_3.47.7-stable.tar.xz \
  -C "$HOME/develop"
ls "$HOME/develop/flutter/bin/flutter"
```

El último comando debe mostrar una ruta que termina en `/develop/flutter/bin/flutter`. `$HOME` representa tu carpeta personal. No uses `sudo` para extraer el SDK. Este recorrido supone que `~/develop/flutter` aún no existía; no extraigas encima de otra instalación.

## 3. Guardar y aplicar PATH

La terminal necesita encontrar los ejecutables de Flutter. Abre el archivo `.bashrc` de tu usuario:

```bash
nano ~/.bashrc
```

`~` es tu carpeta personal; el punto inicial de `.bashrc` forma parte del nombre. En nano pulsa **Alt+/** para ir al final. En una línea nueva añade **una sola vez**, conservando todo el contenido anterior:

```bash
export PATH="$HOME/develop/flutter/bin:$PATH"
```

Esta línea agrega la carpeta `bin` de Flutter y conserva las rutas anteriores después de los dos puntos.

1. Pulsa **Ctrl+O** para guardar.
2. Confirma con **Enter** el nombre del archivo `.bashrc` que muestra nano.
3. Pulsa **Ctrl+X** para salir.
4. Ya en la terminal, aplica y comprueba:

```bash
source ~/.bashrc
command -v flutter
flutter --version
dart --version
```

La ruta debe terminar en `/develop/flutter/bin/flutter`. Deben aparecer Flutter 3.47.7 y Dart 3.13.5. Si no encuentra el comando, revisa la carpeta extraída y la línea de `.bashrc`, vuelve a ejecutar `source ~/.bashrc` y repite las comprobaciones antes de continuar.

## 4. Instalar VS Code y configurar el SDK

Continúa en la misma terminal, dentro de Descargas:

```bash
curl -fL \
  'https://update.code.visualstudio.com/latest/linux-deb-x64/stable' \
  -o code.deb
```

Cuando termine:

```bash
sudo apt install ./code.deb
code
```

Si apt pide confirmar, acepta para continuar. `./code.deb` es el archivo de la carpeta actual.

En VS Code abre Extensiones con **Ctrl+Shift+X**. Busca **Flutter**, selecciona el publicador **Dart Code** (identificador `Dart-Code.flutter`) y pulsa **Instalar**. También instala la extensión Dart. Espera a que termine.

En la terminal de Ubuntu obtén la ruta real del SDK:

```bash
printf "%s\n" "$HOME/develop/flutter"
```

Copia la salida completa. En VS Code abre Configuración con **Ctrl+,**, selecciona **Usuario / User** y busca:

```text
@id:dart.flutterSdkPath
```

En **Dart: Flutter Sdk Path**, pega la ruta anterior. Termina en `flutter`, sin `bin`. El ajuste se guarda automáticamente. Cierra VS Code y vuelve a abrirlo desde la terminal con `code`.

## 5. Comprobar el destino Linux

En la terminal de Ubuntu:

```bash
flutter config --enable-linux-desktop
flutter doctor -v
flutter devices
```

Busca **Linux toolchain** con marca de verificación y **Linux (desktop)** en dispositivos. Si hay errores en Linux toolchain, revisa los paquetes y repite el diagnóstico. Los avisos de Android o Chrome corresponden a otros destinos; este episodio ejecuta una app Linux.

## 6. Descargar el proyecto desde GitHub

1. Abre la [raíz del repositorio](https://github.com/esmarr58/tutoriales).
2. Pulsa **Code / Código → Download ZIP / Descargar ZIP**. No necesitas una cuenta.
3. En las descargas de Firefox, junto a `tutoriales-main.zip`, pulsa **Mostrar en carpeta / Show in Folder**.
4. En Archivos, clic derecho en el ZIP → **Extraer aquí / Extract Here**.
5. Selecciona la carpeta extraída `tutoriales-main` y pulsa **Ctrl+X**.
6. Abre tu carpeta personal. Crea **Proyectos** (con P mayúscula), o abre esa carpeta si ya existe.
7. Dentro de Proyectos pulsa **Ctrl+V**. Si ya hay otra carpeta `tutoriales-main`, consérvala y evita reemplazar trabajo existente.
8. Entra a `tutoriales-main`, `flutter-dart`, `01-entorno-ubuntu-2604` y `hola_flutter`.

Ruta del recorrido:

```text
~/Proyectos/tutoriales-main/flutter-dart/01-entorno-ubuntu-2604/hola_flutter
```

En VS Code elige **Archivo / File → Abrir carpeta / Open Folder** y selecciona **hola_flutter**. Esa carpeta contiene `pubspec.yaml`, `lib` y `test`. Si pide confianza, confirma solo la carpeta de este ejemplo. Abre `lib/main.dart`.

El código es la plantilla inicial del contador Flutter; en este episodio se comprueba el entorno, no se desarrolla el contador desde cero.

## 7. Comprobar y ejecutar

En VS Code abre **Terminal → Nueva terminal / New Terminal**:

```bash
pwd
ls pubspec.yaml
```

La ruta debe terminar en `/hola_flutter` y el segundo comando debe mostrar `pubspec.yaml`. Si no existe, vuelve a **Abrir carpeta** y elige la raíz correcta del proyecto.

Ejecuta uno por uno:

```bash
flutter pub get
flutter analyze
flutter test
```

Espera a que termine cada comando. El análisis debe terminar con **No issues found!** y la prueba con **All tests passed!**.

Antes de ejecutar, predice qué número mostrará al iniciar y qué ocurrirá después de un clic en **+**. Luego ejecuta:

```bash
flutter run -d linux
```

La primera compilación puede tardar. Cuando abra la ventana, el contador debe comenzar en **0**. Pulsa una vez **+**: debe pasar a **1**. Para salir, regresa a la terminal donde corre Flutter y pulsa **q**; vuelve el indicador de la terminal.

## Reto

Abre una terminal nueva en VS Code, comprueba la raíz del proyecto y ejecuta otra vez sin reinstalar nada. En una ejecución nueva, tres pulsaciones de **+** deben dejar el contador en **3**. Criterios: Flutter disponible en la terminal nueva, proyecto correcto y contador de 0 a 3.

Referencias: [Flutter manual](https://docs.flutter.dev/install/manual), [herramientas Linux](https://docs.flutter.dev/platform-integration/linux/setup), [VS Code en Linux](https://code.visualstudio.com/docs/setup/linux), [ajustes Dart Code](https://dartcode.org/docs/settings/#dartfluttersdkpath), [atajos de nano](https://www.nano-editor.org/dist/latest/cheatsheet.html), [descargar archivos de GitHub](https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives).
