# Práctica 5 · Tu robot obedece cuatro colores (proyecto de partida)

Diseño de Interfaces (I7262), 2026B; parte de la serie «Curso de Qt Creator · Qt y OpenCV», después del
[#8](../08-conduccion-autonoma/). Proyecto `robot_colores` (Qt Widgets + CMake + OpenCV, escritorio y Android).
Es el programa 08 de la serie (`08-conduccion-autonoma`) reducido a lo que pide la Práctica 5: el teléfono va
montado en el robot real, su cámara ve una hoja de color y la aplicación le manda la orden a la ESP32-S3.
**Ya funciona**: en la práctica no se programa desde cero; se pone a andar, se calibra y se demuestra.

```
cámara trasera → QVideoSink → QImage → cv::Mat ─→ mirarZona()  ─→ Zona (color, HSV del centro)
                                                    │ zona.cpp (OpenCV) · color.cpp (C++ puro)
QTimer 125 ms → decidir() → MaquinaColores::paso(color) → Accion → enviarAccion() → QWebSocket → robot
                              fsm_p5.cpp (C++ puro)                    JSON «mover»
```

Diagramas: [diagrama-bloques-p5.pdf](diagrama-bloques-p5.pdf) y [diagrama-estados-p5.pdf](diagrama-estados-p5.pdf).
Qué cambió respecto al 08: [CAMBIOS.md](CAMBIOS.md).

## Qué hace

| Hoja en el recuadro | Orden | JSON que se manda (cada 125 ms mientras el color siga firme) |
|---|---|---|
| verde | avanzar | `{"tipo":"mover","accion":"avanzar","ms":300,"vel":30}` |
| azul | izquierda | `{"tipo":"mover","accion":"izquierda","ms":120,"vel":30}` |
| amarilla | derecha | `{"tipo":"mover","accion":"derecha","ms":120,"vel":30}` |
| roja, o ninguna | parar | `{"tipo":"mover","accion":"parar"}` (una sola vez) |

- **Zona.** Se mira el tercio central de la imagen. Un color cuenta si llena al menos el 40 % de la zona. Sobre
  la imagen se escribe `color -> orden  HSV centro h s v` (el HSV del píxel central, no un promedio) y el nombre
  del alumno (constante `ALUMNO`, al inicio de `mainwindow.cpp`).
- **Máquina de estados** (`fsm_p5.h`): `ESPERA`, `OBEDECER` y `PARO`. Un color es «firme» tras verlo 3 ciclos
  seguidos (375 ms); lo mismo vale para «ninguno»: al quitar la hoja, el robot para 3 ciclos después. Sin video
  por más de 1 s o sin conexión, pasa a `PARO` y solo sale con el botón Iniciar. La etiqueta de decisión tiene siempre tres renglones:
  estado y color firme, orden y mensajes por segundo, y el robot (conectado, sin conectar o el motivo del paro).
- **Aprender.** Con la hoja llenando el recuadro, tocar el botón de su color cambia el rango de ese color a lo que
  mide el píxel central (H ± 12; S y V mínimas 70 y 90 por debajo de lo medido, nunca menos de 50). No hace falta
  recompilar para calibrar. Lo aprendido se pierde al cerrar la aplicación.
- **Rangos de partida** (`color.cpp`): amplios, pensados para hojas de papel mate. **No se midieron en un
  teléfono**: son un punto de partida, y por eso existe «Aprender».
- `"vel":30` lo ignora la ESP32-S3 (sus motores son todo o nada); va en el mensaje porque es el mismo `mover`
  del programa 08 en modo «Robot real», y el robot virtual sí lo usa.

## Usarlo

1. Qt Creator → *Open Project* → `CMakeLists.txt`, kit de Android de tu teléfono (o Desktop). Necesita Qt
   Multimedia y Qt WebSockets en el kit, y OpenCV como en los programas 07 y 08.
2. Escribe tu nombre en `ALUMNO` (`mainwindow.cpp`), compila y ejecuta.
3. Fuente `0` → **Encender**. Robot `ws://IP-de-tu-ESP32:81` → **Conectar**. **Iniciar**.

## Probar sin teléfono y sin robot

```bash
g++ -std=c++17 -I. pruebas/prueba_logica.cpp color.cpp fsm_p5.cpp -o prueba_logica
./prueba_logica                                  # 47 comprobaciones correctas, 0 fallas

pip install opencv-python websockets
python pruebas/prueba_escritorio.py build/robot_colores      # termina con «RESULTADO: TODO PASA»
```

`prueba_escritorio.py` levanta en `127.0.0.1` un video MJPEG de hojas de color sintéticas (puerto 8772) y un robot
de mentira por WebSocket (puerto 8771), lanza el programa con `--fuente … --robot … --auto` y comprueba: las
cuatro hojas y su `mover`, `ms` y `vel`, 8 mensajes por segundo, «parar» una sola vez, paro al quitar la hoja,
paro al cortarse el video, y el botón Aprender (`--aprender verde`) con una hoja que el punto de partida no
reconoce. No usa el robot virtual.

## Probado en

| Dónde | Qué | Resultado |
|---|---|---|
| Ubuntu1 (VM), Qt 6.12.0, OpenCV 4.6.0, kit Desktop (16-nov-2026) | compila sin errores ni avisos; `prueba_logica`; `prueba_escritorio.py`, 6 corridas completas | 47 comprobaciones, 0 fallas; 6 de 6 corridas con «TODO PASA»: 22 mensajes por tramo, 8.0 por segundo, `parar` 0.38 s después de quitar la hoja y 1.01 s después de cortarse el video |
| Ubuntu1, kit Qt 6.12.0 for Android x86_64, OpenCV Android SDK 4.14.0; emulador API 35 por software | compila sin errores ni avisos (APK de depuración de 68.2 MB); instala y abre; la interfaz y los cuatro botones «Aprender» caben en vertical; al pulsar **Encender**, Android pide el permiso de cámara | hasta ahí, correcto. **La cámara no se pudo probar:** ese día el emulador no ofreció ninguna cámara («No se encontró ninguna cámara.») y después dejó de responder |
| Windows 11, Qt 6.10.2 (MinGW 13.1), OpenCV 4.10.0 (16-nov-2026) | compila sin avisos (`-Wall -Wextra`); `prueba_logica`; `prueba_escritorio.py`, 6 corridas | 47 comprobaciones, 0 fallas; 5 de 6 corridas con «TODO PASA». En la primera corrida tras compilar, la hoja verde sintética no produjo `avanzar`; no se repitió ni en Windows ni en Ubuntu, y no se encontró la causa |

**Todavía no se ha probado:** la aplicación en un teléfono; la cámara de Android (ni real ni la del emulador); la
clasificación de colores y «Aprender» corriendo en Android; hojas reales (los rangos de partida); el robot físico
(ESP32-S3), incluidos los 300 ms de avance y los 120 ms de giro. Que la ESP32-S3 ignora `vel` se comprobó leyendo
`practica4.ino`, no en la tarjeta.
