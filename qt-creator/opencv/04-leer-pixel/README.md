# Qt y OpenCV #4 · Lee el valor de un píxel con un clic o un toque

Un `eventFilter` en la etiqueta de la imagen convierte la posición del clic (en Android, el toque llega como clic) a coordenadas de la imagen original y muestra B, G, R y un cuadro con ese color.

**Prueba esperada.** Círculo rojo → «(170, 130)  B=0 G=0 R=255»; círculo mixto → «B=200 G=120 R=40»; fuera de la imagen → «Fuera de la imagen». Igual en el teléfono.

Proyecto completo `visor_opencv` (estado final del episodio). Ábrelo en Qt Creator con los kits **Desktop
Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**; requisitos y pasos en el [README de la serie](../README.md).
Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
