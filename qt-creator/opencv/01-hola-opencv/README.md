# Qt y OpenCV #1 · Un proyecto, dos kits: Hola OpenCV

Crea `visor_opencv` con el asistente de Qt Creator (Qt Widgets, CMake, sin formulario `.ui`), lo enlaza con OpenCV con `find_package` y un bloque `if(ANDROID)`, y muestra la versión con `cv::getVersionString()`.

**Prueba esperada.** Escritorio: «Hola, OpenCV 4.6.0». Emulador: «Hola, OpenCV 4.14.0».

Proyecto completo `visor_opencv` (estado final del episodio). Ábrelo en Qt Creator con los kits **Desktop
Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**; requisitos y pasos en el [README de la serie](../README.md).
Qué cambió respecto al asistente: [CAMBIOS.md](CAMBIOS.md).
