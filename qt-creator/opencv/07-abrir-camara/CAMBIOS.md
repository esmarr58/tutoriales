# Cambios respecto al 06

Proyecto nuevo, `camara_opencv` (ya no `visor_opencv`): no carga imágenes de recursos, las toma de la cámara.

- `CMakeLists.txt`: componente `Multimedia` en `find_package` y `Qt::Multimedia` en `target_link_libraries`;
  `qt_add_android_permission(... android.permission.CAMERA)` dentro de `if(ANDROID)`. Sin `.qrc` ni `CMAKE_AUTORCC`.
- `puente.h` / `puente.cpp`: los mismos del 02–06 (`aMat`, `aQImage`).
- `mainwindow.*`: `QMediaCaptureSession` + `QVideoSink` como miembros; `QCamera` creada al encender;
  `pedirPermiso()` con `QCameraPermission`; `alLlegarCuadro()` guarda el último cuadro y `procesarPendiente()`
  lo convierte (descarte de cuadros); medición de cuadros por segundo con `QElapsedTimer`.
- `main.cpp`: sin cambios.
