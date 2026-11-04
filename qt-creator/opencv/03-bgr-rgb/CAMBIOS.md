# 03 · BGR contra RGB · cambios respecto al 02

- `recursos.qrc`: se agrega `muestras/colores_bgr.png` (círculos rojo, verde, azul puros y uno mixto).
- `mainwindow.cpp`: carga `colores_bgr.png` en vez de `formas.png`.
- Nuevo `botonDibujar` («Dibujar con OpenCV», 64 px de alto) y su `connect()` con puntero a función
  miembro: `connect(botonDibujar, &QPushButton::clicked, this, &MainWindow::dibujarConOpenCV)`.
- `dibujarConOpenCV()`: `cv::circle` con `cv::Scalar(0, 0, 255)` (rojo en BGR), anillo alrededor del
  círculo rojo (centro 170, 130).
- `puente.cpp` (**la corrección**): `aMat` usa `cv::cvtColor(…, cv::COLOR_RGB2BGR)` en vez de `clone()`
  (cvtColor también copia) y `aQImage` usa `cv::COLOR_BGR2RGB` antes de envolver. Incluye
  `<opencv2/imgproc.hpp>`.

Para provocar el error en el video: el `mainwindow` del 03 con el `puente.cpp` del 02 (copia en
`variantes/puente_sin_cvtcolor.cpp`). Con él, el anillo «rojo» sale **azul** alrededor del círculo
rojo; con el puente corregido sale **rojo**.
