# Cambios respecto al 08

Proyecto nuevo, `robot_colores`: es el programa 08 (`08-conduccion-autonoma`) reducido a lo que pide la
Práctica 5. El 08 maneja un coche entre dos rayas; este obedece cuatro hojas de color. Se conserva el
esqueleto: cámara → `cv::Mat` → percepción → máquina de estados → reloj de 125 ms → JSON por WebSocket.

- `CMakeLists.txt`: proyecto y ejecutable `robot_colores`; mismos componentes de Qt (Multimedia, WebSockets) y
  de OpenCV (`core`, `imgproc`); lista de archivos nueva; `prueba_logica` enlaza `color.cpp` y `fsm_p5.cpp`.
- `puente.h` / `puente.cpp`: sin cambios (`aMat`, `aQImage`).
- **Se quitaron** `franja.*` (no hay rayas), `percepcion.*` y `fsm.*`.
- `color.h` / `color.cpp`: mismos cuatro colores y `clasificarHsv()`. Cambia:
  - los rangos ya no son constantes: `fijarRangos()` los cambia en marcha y `rangosAprendidos(h, s, v)` calcula
    el rango alrededor de un tono (dos rangos si cruza el 0);
  - rangos de partida más amplios, pensados para papel (no medidos en un teléfono);
  - se quitaron `esDisco()` y `colorDominante()`: una hoja no es un disco.
- **Nuevo** `zona.h` / `zona.cpp` (todo el OpenCV): `mirarZona()` mira el tercio central, dice qué color lo llena
  al menos al 40 % y mide el HSV del píxel central; `mascaraDe()` es la del 08; `dibujarZona()` escribe sobre el
  cuadro.
- **Nuevo** `fsm_p5.h` / `fsm_p5.cpp` (C++ puro): `MaquinaColores` con `ESPERA`, `OBEDECER` y `PARO`
  (`enum class` + `switch`), el antirrebote de 3 ciclos del 08, la tabla `ordenDe()` y `movimientoDe()`.
- `mainwindow.*`:
  - constante `ALUMNO` al inicio de `mainwindow.cpp`: el nombre sale escrito sobre la imagen;
  - `procesarPendiente()` llama a `mirarZona()` en lugar de `percibir()`;
  - `decidir()` llama a `maquina.paso(color)` y `enviarAccion()` manda siempre `mover` (con `ms` y `vel`),
    como el modo «Robot real» del 08; ya no existe `conducir`;
  - se quitaron el deslizador «Umbral» y las casillas «Rayas oscuras» y «Robot real»;
  - cuatro botones **Aprender** (verde, rojo, azul, amarillo) para calibrar sin recompilar;
  - «Fuente» arranca en `0` (la cámara trasera) y la etiqueta de decisión dice estado, color firme y orden.
    Esa etiqueta se escribe siempre en tres renglones cortos: en el teléfono, un renglón largo se partía y
    salía recortado al abrir.
- `main.cpp`: opción `--aprender <color>`, además de `--fuente`, `--robot` y `--auto`.
- `pruebas/`: `prueba_logica.cpp` y `prueba_escritorio.py` (video de hojas sintéticas y robot de mentira).
- `diagrama-bloques-p5.*` y `diagrama-estados-p5.*`.
