# Cambios respecto al 07

Mismo proyecto, `camara_opencv`. El 07 abre la cámara (o el stream) y muestra el cuadro; el 08 además lo
analiza, decide y le manda comandos al robot.

- `CMakeLists.txt`: componente `WebSockets` en `find_package` y `Qt::WebSockets` en `target_link_libraries`;
  los archivos nuevos en `qt_add_executable`; un segundo ejecutable, `prueba_logica`, que no enlaza Qt ni
  OpenCV (solo fuera de Android). El permiso `android.permission.INTERNET` del 07 también sirve para el
  WebSocket.
- `puente.h` / `puente.cpp`: los mismos del 02–07 (`aMat`, `aQImage`), sin cambios.
- **Nuevos, C++ puro (sin Qt ni OpenCV):**
  - `franja.h` / `franja.cpp`: `analizarFranja()` halla las dos rayas en el perfil de la franja y calcula
    centro, error y ancho.
  - `color.h` / `color.cpp`: rangos HSV de los cuatro colores, `clasificarHsv()`, `esDisco()` (área, aspecto y
    llenado) y `colorDominante()` (el rojo manda).
  - `fsm.h` / `fsm.cpp`: `MaquinaConduccion` (`enum class Estado`, `paso()` con un `switch`), antirrebote del
    color, memoria del rojo atendido, y `aRobotReal()` para traducir la orden a «mover».
- **Nuevo, todo el OpenCV:** `percepcion.h` / `percepcion.cpp`: `perfilDeFranja()` (`cvtColor` + `reduce`),
  `mascaraDe()` (`inRange` + `bitwise_or`), `percibir()` (`connectedComponentsWithStats`) y `dibujarVista()`.
- `mainwindow.*`:
  - Se quitó la casilla «Escala de grises»; `procesarPendiente()` ahora llama a `percibir()` y dibuja la
    franja, los bordes, el centro del carril y la paleta sobre el cuadro.
  - Campo «Robot», botones **Conectar** e **Iniciar / Detener**, deslizador «Umbral», casillas «Rayas oscuras»
    y «Robot real», y una etiqueta con el estado, la marcha, la paleta y los mensajes por segundo.
  - `QWebSocket robot` con latido cada 5 s; `QTimer relojDecision` de 125 ms → `decidir()` → `enviarOrden()`.
  - Vigilancias: sin conexión o sin cuadros nuevos por más de 1 s, la máquina pasa a `PARO`.
  - El campo «Fuente» arranca con `http://127.0.0.1:8080/stream.mjpg` (antes `0`).
- `main.cpp`: opciones `--fuente`, `--robot` y `--auto` para probar desde la terminal.
- `pruebas/`: `prueba_logica.cpp`, `prototipo.py`, `vuelta_simulador.py`, `exportar_datos.py`, `cuadros/` (12
  cuadros del mapa Circuito del robot virtual) y `datos/`.
- Se quitó `prueba_stream/servidor_mjpeg.py`: el video de prueba ahora es el del robot virtual.
