# Qt y OpenCV en escritorio y Android

Serie «Curso de Qt Creator · Qt y OpenCV». Un mismo proyecto, `visor_opencv` (Qt Widgets + CMake, C++17,
interfaz construida por código en `MainWindow`), crece de un video al siguiente: cada carpeta es el estado
final de su episodio y compila **sin cambios** con los dos kits de Qt Creator. `CAMBIOS.md` de cada carpeta
dice qué cambió respecto al episodio anterior.

## Requisitos

| | Escritorio (kit «Desktop Qt 6.12.0») | Android (kit «Qt 6.12.0 for Android x86_64») |
|---|---|---|
| Qt | Qt 6.12.0 + Qt Creator 20 | lo mismo + SDK/NDK de Android (video [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs) y scripts de [`../android-sdk-hola-mundo`](../android-sdk-hola-mundo/)) |
| OpenCV | **4.6.0** de Ubuntu 24.04: `sudo apt install libopencv-dev` (comprobar con `pkg-config --modversion opencv4`) | **OpenCV Android SDK 4.14.0**: `opencv-4.14.0-android-sdk.zip` de https://github.com/opencv/opencv/releases/tag/4.14.0 descomprimido en `~/opencv` (debe existir `~/opencv/OpenCV-android-sdk/sdk/native/jni/OpenCVConfig.cmake`) |
| Dispositivo | — | emulador `Qt_Hola_API35` (x86_64) |

Si descomprimiste el SDK en otra carpeta, cambia la ruta de `OpenCV_DIR` en `CMakeLists.txt`.

## Cómo abrirlo con los dos kits

1. Descarga la carpeta del episodio y abre su `CMakeLists.txt` en Qt Creator (File → Open File or Project).
2. En *Configure Project* marca **Desktop Qt 6.12.0** y **Qt 6.12.0 for Android x86_64** → *Configure Project*.
3. Escritorio: elige el kit Desktop en el selector (barra lateral, sobre Run) y pulsa Ctrl+R.
4. Android: inicia el emulador, elige el kit Android x86_64 (el teléfono con punto **verde** = emulador
   detectado) y pulsa Ctrl+R. La primera vez tarda en compilar e instalar; la aplicación tarda unos segundos
   más en dibujarse.

## Cómo se enlaza OpenCV (`CMakeLists.txt`)

```cmake
set(CMAKE_AUTORCC ON)            # compila recursos.qrc (la plantilla de Qt 6 no lo activa)

if(ANDROID)
    set(OpenCV_DIR "$ENV{HOME}/opencv/OpenCV-android-sdk/sdk/native/jni")
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
target_link_libraries(visor_opencv PRIVATE Qt::Core Qt::Widgets ${OpenCV_LIBS})
```

- En Android el SDK elige solo la carpeta de la ABI del kit (`x86_64`) y enlaza las bibliotecas **estáticas**:
  APK de depuración de ~41 MB, sin `libopencv_java4.so` ni `QT_ANDROID_EXTRA_LIBS`.
- `cv::imread` no lee rutas `:/` de los recursos de Qt: las imágenes se cargan con `QImage` y pasan a
  `cv::Mat` con el puente (`puente.h/.cpp`).

## Episodios

- [Qt y OpenCV #2 · De QImage a cv::Mat: muestra tu primera imagen](02-de-qimage-a-mat/)
- [Qt y OpenCV #3 · ¿Colores al revés? BGR contra RGB](03-bgr-rgb/)
- [Qt y OpenCV #4 · Lee el valor de un píxel con un clic o un toque](04-leer-pixel/)
- [Qt y OpenCV #5 · Escala de grises, brillo y contraste con un deslizador](05-gris-brillo-contraste/)
- [Qt y OpenCV #6 · Práctica: un inspector de imágenes](06-inspector-imagenes/)

Las imágenes de `muestras/` las genera nuestro propio código (formas y colores con valores conocidos y una
imagen sintética de «células»); se pueden usar libremente. OpenCV se distribuye con licencia Apache 2.0
(https://opencv.org/license/).
