# 05 · Escala de grises, brillo y contraste · cambios respecto al 04

- `puente.cpp`: `aQImage` acepta `cv::Mat` de **1 canal** (`QImage::Format_Grayscale8`).
- `mainwindow.h`: `casillaGris` (QCheckBox), `deslizadorContraste` y `deslizadorBrillo` (QSlider) con sus
  etiquetas, `aplicarAjustes()` y un segundo `cv::Mat resultado` (lo que se ve). `imagen` queda como la
  original.
- `aplicarAjustes()`: `cv::cvtColor(imagen, base, cv::COLOR_BGR2GRAY)` si la casilla está marcada y
  `base.convertTo(resultado, -1, alpha, beta)` con alpha = valor / 10 (0.0–3.0) y beta = −100…100.
- Tres `connect()` a la misma función (`toggled`, `valueChanged`, `valueChanged`). Los deslizadores tienen
  56 px de alto y una hoja de estilo con perilla de 44 px para el dedo.
- `mostrarImagen()` y `leerPixel()` usan `resultado`: en gris se lee «gris=V».
- `cargarMuestra()` y `dibujarConOpenCV()` terminan con `aplicarAjustes()`.

Prueba observable: con la casilla, «gris=76» en el círculo rojo (0.299·255); contraste a 2.0 → gris 152;
brillo +100 con alpha 1.0 → 176; el fondo blanco se queda en 255 (saturación).
