# Qt y OpenCV #5 · Escala de grises, brillo y contraste con un deslizador

Casilla «Escala de grises» (`cvtColor` BGR2GRAY) y dos `QSlider` para `convertTo(resultado, -1, alpha, beta)`: contraste alpha 0.0–3.0 y brillo beta −100…100.

**Prueba esperada.** Gris del círculo rojo = 76; con alpha 2.0 → 152; con beta +100 → 176; el blanco se queda en 255.

Proyecto completo `visor_opencv` (estado final del episodio). Ábrelo en Qt Creator con los kits **Desktop
Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**; requisitos y pasos en el [README de la serie](../README.md).
Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
