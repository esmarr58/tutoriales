# Qt y OpenCV #3 · ¿Colores al revés? BGR contra RGB

Carga `colores_bgr.png` y el botón «Dibujar con OpenCV» dibuja un anillo con `cv::Scalar(0, 0, 255)` (rojo en BGR). El puente corrige el orden con `cv::cvtColor` (RGB2BGR y BGR2RGB).

**Prueba esperada.** Con el puente del 02 (`variantes/puente_sin_cvtcolor.cpp` en lugar de `puente.cpp`) el anillo sale **azul**; con el puente de esta carpeta sale **rojo**, igual que el círculo.

Proyecto completo `visor_opencv` (estado final del episodio). Ábrelo en Qt Creator con los kits **Desktop
Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**; requisitos y pasos en el [README de la serie](../README.md).
Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
