# Qt y OpenCV #2 · De QImage a cv::Mat: muestra tu primera imagen

Crea `puente.h/.cpp` (QImage ↔ cv::Mat con copia de los píxeles), carga `formas.png` de los recursos (`.qrc`) y la muestra escalada sin deformar, con su ancho, alto y canales.

**Prueba esperada.** Escritorio y emulador: las 12 formas y «formas.png · 640 × 480 · 3 canales».

Proyecto completo `visor_opencv` (estado final del episodio). Ábrelo en Qt Creator con los kits **Desktop
Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**; requisitos y pasos en el [README de la serie](../README.md).
Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
