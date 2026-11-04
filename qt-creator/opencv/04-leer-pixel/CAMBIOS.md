# 04 · Leer un píxel con un clic o un toque · cambios respecto al 03

- `mainwindow.h`: `eventFilter()` (override), `leerPixel(QPoint)`, `muestraColor` y `etiquetaPixel`.
- `construirInterfaz()`: `etiquetaImagen->installEventFilter(this)`; fila nueva (`QHBoxLayout`) con un
  cuadro de 64 × 64 pintado con el color leído y el texto «(x, y)  B=… G=… R=…».
- `eventFilter()`: atiende `QEvent::MouseButtonPress` sobre la etiqueta de la imagen. En Android el toque
  llega como clic del ratón (Qt lo sintetiza), así que el mismo código sirve en los dos kits.
- `leerPixel()`: convierte la posición en la etiqueta a coordenadas de la imagen original (escala de
  `KeepAspectRatio` y márgenes del centrado), comprueba que caiga dentro y lee
  `imagen.at<cv::Vec3b>(y, x)` (fila primero). El cuadro de color usa `rgb(R, G, B)` de Qt: el orden
  vuelve a importar.

Prueba observable: clic/toque en el círculo rojo → «(x, y)  B=0 G=0 R=255»; en el mixto
«B=200 G=120 R=40»; fuera de la imagen → «Fuera de la imagen».
