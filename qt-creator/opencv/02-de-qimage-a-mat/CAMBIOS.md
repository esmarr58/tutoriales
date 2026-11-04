# 02 · De QImage a cv::Mat · cambios respecto al 01

Archivos nuevos:

- `puente.h` / `puente.cpp` (Add New… → C++ Source/Header): `aMat(QImage)` y `aQImage(cv::Mat)`.
  Los dos **copian** los píxeles (`clone()` / `copy()`): el resultado no depende de que la imagen de
  origen siga viva. Formato fijo `QImage::Format_RGB888` (3 bytes por píxel) y `bytesPerLine()` como paso
  de fila del `cv::Mat`.
- `recursos.qrc` (Add New… → Qt Resource File) con el prefijo `/` y `muestras/formas.png`.
- `muestras/formas.png` (640 × 480, 12 formas, generada por `herramientas/generar_material.py`).

Cambios:

- `CMakeLists.txt`: `puente.cpp puente.h recursos.qrc` en `qt_add_executable` (Qt Creator los agrega solo
  al crear los archivos) y **`set(CMAKE_AUTORCC ON)`**: la plantilla de Qt 6 de Qt Creator 20 no lo activa y,
  sin él, el `.qrc` no se compila y la imagen no abre («No se pudo abrir :/muestras/formas.png», visto en la
  VM). El bloque de OpenCV no cambia.
- `mainwindow.h/.cpp`: la etiqueta de la versión se sustituye por `etiquetaImagen` (imagen escalada con
  `KeepAspectRatio`, nítida con `devicePixelRatioF()`) y `etiquetaEstado` («formas.png · 640 × 480 ·
  3 canales»). Métodos `construirInterfaz()`, `cargarMuestra()`, `mostrarImagen()` y `resizeEvent()`.

Prueba observable: escritorio y emulador muestran las 12 formas y «formas.png · 640 × 480 · 3 canales».

Nota para el 03: este puente deja los bytes en orden **R, G, B** dentro del `cv::Mat`, aunque OpenCV
supone **B, G, R**. Se ve bien porque `aQImage` los devuelve igual; el 03 lo pone en evidencia.
