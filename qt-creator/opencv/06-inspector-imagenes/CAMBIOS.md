# 06 · Práctica: inspector de imágenes · cambios respecto al 05

- `recursos.qrc`: se agrega `muestras/celulas.png` (640 × 360, fluorescencia sintética).
- Selector de muestra: tres `QPushButton` marcables (`setCheckable`) en un `QButtonGroup` («Formas»,
  «Colores», «Células», 64 px, el elegido en azul) y una lista `rutas` en `mainwindow.h`. `connect` con
  **lambda** a `QButtonGroup::idClicked`. El plan proponía un QComboBox: en el emulador por software su
  lista se abre al presionar y se cierra sola al soltar (Qt la cierra si la liberación llega después de
  `doubleClickInterval`), tanto con `adb input tap` como con clic del ratón en la ventana del emulador.
- `botonRestablecer`: casilla sin marcar, alpha 1.0 y beta 0.
- Se quita `botonDibujar` (era la prueba del 03).
- Diseño que se adapta (`QBoxLayout` + `acomodar()` en `resizeEvent`): ventana ancha → imagen a la
  izquierda y panel a la derecha (`setStretch` 3:2); teléfono vertical → imagen arriba y panel abajo.
- El píxel elegido se recuerda en coordenadas de la imagen (`cv::Point pixelElegido`) y `mostrarPixel()`
  lo vuelve a leer al final de `aplicarAjustes()`: al mover un deslizador o pulsar «Restablecer», el
  valor se actualiza solo. `cargarMuestra()` lo olvida.
- `etiquetaImagen->setMinimumSize(120, 90)` (antes 200 × 150): con el mínimo anterior, el arranque en
  horizontal fijaba un ancho mínimo de 410 puntos y en el teléfono (393 puntos) el panel se salía por la
  derecha (comprobado en el emulador).
- Título de la ventana: «Inspector de imágenes».

Reto con criterios: en las tres muestras, el estado dice nombre · ancho × alto · canales correctos
(640 × 480, 640 × 480, 640 × 360); el píxel del círculo rojo da B=0 G=0 R=255 y en gris 76; con contraste
2.0 el gris del círculo rojo es 152; «Restablecer» devuelve la imagen original.
