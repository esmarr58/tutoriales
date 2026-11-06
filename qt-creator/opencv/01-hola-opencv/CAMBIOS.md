# 01 · Hola OpenCV · cambios sobre lo que genera el asistente

Asistente de Qt Creator 20: *Qt Widgets Application*, **CMake**, clase `MainWindow` **sin** «Generate form», kits **Desktop Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**.

- `CMakeLists.txt`: bloque `if(ANDROID) set(OpenCV_DIR …)`, `find_package(OpenCV REQUIRED COMPONENTS core imgproc)` y `${OpenCV_LIBS}` en `target_link_libraries`.
- `mainwindow.h`: `#include <QLabel>` y `QLabel *etiquetaVersion`.
- `mainwindow.cpp`: letra de 20 pt y una etiqueta con `cv::getVersionString()`.
