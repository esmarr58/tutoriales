# Qt y OpenCV #8 · Conducción autónoma con una máquina de estados

El proyecto `camara_opencv` del [#7](../07-abrir-camara/) ya abre la cámara del robot virtual por streaming.
Aquí además **decide y maneja**: busca las dos rayas del carril en una franja horizontal, reconoce las paletas
de color de los postes y manda comandos JSON por WebSocket.

```
stream MJPEG o cámara → QVideoSink → QImage → cv::Mat ─→ percibir()  ─→ Vista (rayas, error, paleta)
                                                          │ franja.cpp, color.cpp (C++ puro)
QTimer 125 ms → decidir() → MaquinaConduccion::paso(Medida) → Orden → enviarOrden() → QWebSocket → robot
                              fsm.cpp (C++ puro)                        JSON «conducir» o «mover»
```

Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).

## Qué hace

- **Percepción 1, las dos rayas.** Recorta una franja de 8 filas al 75 % del alto (fila 360 de 480), la pasa a
  gris y promedia cada columna (`cv::reduce`): queda un perfil de `ancho` números. Del centro hacia afuera
  busca la primera columna con gris ≥ 170 a cada lado; el centro del carril es el punto medio y el error es
  ese centro menos el centro de la imagen. Con una sola raya estima el centro con el último ancho medido (al
  inicio, 47 % del ancho); un «carril» de menos del 25 % o de más del 80 % del ancho no se acepta (la meta a
  cuadros, la bifurcación).
- **Percepción 2, las paletas.** En la zona del 30 % al 55 % del alto: BGR → HSV, una máscara por color con
  `cv::inRange` (el rojo une dos rangos) y `cv::connectedComponentsWithStats`. Una mancha es paleta si mide
  300 px o más, su caja es casi cuadrada (aspecto 0.70 a 1.40) y está llena (≥ 0.60): así no cuentan el anillo
  de las señales de velocidad, los pianos ni las bandas del piso.

  | Color | H (0–179) | S mínima | V mínima |
  |---|---|---|---|
  | rojo | 0–10 y 170–179 | 120 | 100 |
  | verde | 50–80 | 150 | 100 |
  | azul | 100–125 | 120 | 100 |
  | amarillo | 20–34 | 180 | 180 |

- **Máquina de estados** (`fsm.h`). Una paleta cuenta tras verla 3 ciclos seguidos (antirrebote).

  | Estado | Qué manda | Sale cuando |
  |---|---|---|
  | `ESPERA` | `mover parar`, una sola vez | botón **Iniciar** → `SEGUIR` |
  | `SEGUIR` | `conducir`: dirección = 0.35 × error (de −100 a 100); acelerador según la marcha | paleta roja → `ALTO`; ninguna raya → `LINEA_PERDIDA` |
  | `ALTO` | `conducir` con freno 100 | a los 3 s → `SEGUIR` en crucero (esa paleta roja ya no vuelve a detenerlo) |
  | `LINEA_PERDIDA` | `conducir` con acelerador 18 y la última dirección | ve una raya → `SEGUIR`; 1.5 s sin rayas → `PARO` |
  | `PARO` | `mover parar`, una sola vez | botón **Iniciar** → `SEGUIR` |

  Desde cualquier estado: **Detener** → `ESPERA`; sin conexión con el robot o más de 1 s sin cuadros nuevos →
  `PARO`.

  | Paleta | Decisión | Comando |
  |---|---|---|
  | roja (o señal de ALTO) | `ALTO`: frena, espera 3 s y sigue | `conducir` con freno 100 |
  | verde | marcha crucero | `conducir` con acelerador 28 |
  | amarilla | marcha lenta | `conducir` con acelerador 22 |
  | azul | marcha rápida | `conducir` con acelerador 34 |

  El significado de las paletas es una decisión de este programa: el robot virtual no lo fija.
- **Envío.** Un `QTimer` de 125 ms decide 8 veces por segundo: nunca un mensaje por cuadro.
  `{"tipo":"conducir","acelerador":28,"direccion":-6,"freno":0,"ms":500}` se repite en cada ciclo (su `ms` es
  una vigilancia: si deja de llegar, el coche frena solo); `{"tipo":"mover","accion":"parar"}` se manda una
  sola vez. Un latido cada 5 s mantiene la conexión y la telemetría.
- **Robot real** (casilla): el firmware del ESP32-S3 no entiende `conducir`. La misma orden se traduce a
  `{"tipo":"mover","accion":"avanzar","ms":300,"vel":30}`, o `izquierda` / `derecha` con `ms` 120 cuando la
  dirección pasa de ±25, también 8 veces por segundo. El coche virtual usa `vel`; sin ese campo acelera al
  100 %.
- **Rayas oscuras** (casilla) y **Umbral** (deslizador): para una pista real de tiras negras sobre piso claro.

## Usarlo

1. Abre el robot virtual en el mapa Circuito (`robotvirtual -- --mapa=circuito`). El panel «Conexión» muestra
   los puertos: comandos en 8081 u 81, video en 8080.
2. Qt Creator → *Open Project* → `CMakeLists.txt`, kits **Desktop Qt 6.12.0** y **Qt 6.12.0 for Android
   x86_64** (requisitos en el [README de la serie](../README.md)). Además de Qt Multimedia necesita **Qt
   WebSockets** en los dos kits (Qt Maintenance Tool → Additional Libraries).
3. Fuente: `http://IP:8080/stream.mjpg` → **Encender**. Robot: `ws://IP:8081` → **Conectar**. **Iniciar**.
   En el emulador de Android, la IP del equipo anfitrión es `10.0.2.2`.
4. Desde la terminal, sin tocar botones:
   `./camara_opencv --fuente http://127.0.0.1:8080/stream.mjpg --robot ws://127.0.0.1:8081 --auto`

**Prueba esperada.** Sobre el video: la franja amarilla, un punto magenta en cada raya, una cruz verde en el
centro del carril y un recuadro magenta en la paleta. Abajo: «SEGUIR · marcha crucero · paleta verde · 8
mensajes/s» y la velocidad. El coche da la vuelta al Circuito, se detiene 3 s en cada paleta roja y en las
señales de ALTO, y cambia de marcha con las otras paletas. Al pulsar **Desconectar**: «PARO: sin conexión con
el robot».

## Probar la lógica sin Qt, sin OpenCV y sin robot

`franja.cpp`, `color.cpp` y `fsm.cpp` no incluyen nada de Qt ni de OpenCV:

```bash
g++ -std=c++17 -I. pruebas/prueba_logica.cpp franja.cpp color.cpp fsm.cpp -o prueba_logica
./prueba_logica pruebas/datos        # 128 comprobaciones correctas, 0 fallas
```

- Casos escritos a mano y `pruebas/datos/`: 12 perfiles y 59 manchas de cuadros del simulador
  (`pruebas/cuadros/`) y 1645 ciclos de una vuelta al Circuito. El C++ debe dar lo mismo que
  `pruebas/prototipo.py` (Python + OpenCV), el espejo de la lógica que manejó el coche al grabar esos datos.
- `pruebas/vuelta_simulador.py`: lazo cerrado en Python contra el juego (`pip install opencv-python
  websocket-client`). `pruebas/exportar_datos.py` regenera `pruebas/datos/`.

## Probado en

Ubuntu1 (VM), Qt 6.12.0, OpenCV 4.6.0 (escritorio) y OpenCV Android SDK 4.14.0 (emulador API 35, x86_64); robot
virtual en Windows, mapa Circuito, cámara por omisión (640 × 480, 30 cuadros/s, realismo 0.5). 16-nov-2026.

| Qué | Resultado |
|---|---|
| Compilar con los dos kits | sin errores ni avisos; APK de depuración de 68.6 MB |
| `prueba_logica` | 128 comprobaciones correctas, 0 fallas |
| Escritorio, `--auto`, 400 s | 1383 m (más de dos vueltas, con la bifurcación), 0 infracciones, máximo 20.4 km/h; 6 altos por vuelta; una vuelta en unos 169 s; 7.9 `conducir` por segundo; 12 a 14 cuadros/s mostrados |
| Escritorio, botones | **Detener**: un solo `mover parar` y el coche frena; **Desconectar**: `PARO` y el coche se detiene solo |
| Escritorio, casilla «Robot real» contra el coche virtual, 60 s | 159 m, 0 infracciones, máximo 18 km/h (`avanzar`, `izquierda` y `derecha` con `vel` 30) |
| Emulador de Android por software | instala, la interfaz cabe en vertical, abre el stream por `http://10.0.2.2:8080/stream.mjpg`, conecta el WebSocket por `ws://10.0.2.2:8081` y dibuja franja, rayas y paleta |

**En el emulador por software no conduce:** llegan de 0.8 a 1.3 cuadros por segundo (como en el #7), se cumple
la vigilancia de 1 s sin video y la máquina pasa a «PARO: sin video». Es el emulador, no el programa.

**Sin probar:** un teléfono Android real; la casilla «Robot real» con el robot físico (ESP32-S3); la casilla
«Rayas oscuras» y el deslizador con una pista real; otros valores de `realismo` de la cámara.
