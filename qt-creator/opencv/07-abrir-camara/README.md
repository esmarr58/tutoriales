# Qt y OpenCV #7 · Abre la cámara en escritorio y Android

Proyecto nuevo `camara_opencv`: abre una cámara o un video por red (streaming) con Qt Multimedia y pasa cada
cuadro a OpenCV.

```
número (0, 1…):  QCamera → QMediaCaptureSession ┐
URL (http://…):  QMediaPlayer ──────────────────┴→ QVideoSink → QVideoFrame → QImage → cv::Mat → (OpenCV) → QLabel
```

- **Fuente**: como en `cv::VideoCapture`, un número es una cámara local (`0` = la primera) y una URL es un stream:
  una cámara IP, una ESP32-CAM (`http://IP:81/stream`), un teléfono con una aplicación de cámara IP o el
  servidor de prueba de este repositorio. Las dos fuentes entregan sus cuadros al mismo `QVideoSink`, así que el
  resto del programa no cambia.

- **Encender / Apagar**, **Cambiar cámara** (si hay más de una: frontal y trasera, o dos webcams).
- **Escala de grises (OpenCV)**: `cv::cvtColor(…, COLOR_BGR2GRAY)` sobre cada cuadro.
- Etiqueta de estado: nombre de la cámara, ancho × alto, canales y cuadros por segundo medidos.
- **Permiso de cámara**: en Android, `QCameraPermission` abre el diálogo del sistema la primera vez; en el
  escritorio ya está concedido. El `CMakeLists.txt` declara `android.permission.CAMERA` con
  `qt_add_android_permission`. El stream no usa la cámara: no pide ese permiso, solo `android.permission.INTERNET`.
- **Descarte de cuadros**: se guarda solo el último cuadro y hay una sola conversión en espera. Sin esto, en un
  equipo lento los cuadros se acumulan, la memoria crece y Android cierra la aplicación.

**Prueba esperada.** Escritorio (Ubuntu): «<nombre de tu webcam> · 640 × 480 · 3 canales · ~15–30 cuadros/s»; con
la casilla, «1 canal» y la imagen en gris. Android: diálogo de permiso, «Mientras se usa la app», y la imagen de la
cámara trasera en el teléfono, girada en vertical (480 × 640). Al apagar, queda el último cuadro y la casilla
lo sigue procesando.

En el emulador por software (sin aceleración, como en una VM) llegan de 0.4 a 1.5 cuadros por segundo: es el
emulador, no el programa: en el escritorio, el mismo programa muestra 15 cuadros/s, los que manda la webcam virtual.

## Probar el streaming sin cámara IP

`prueba_stream/servidor_mjpeg.py` publica un patrón de colores con reloj en MJPEG por HTTP (necesita `ffmpeg`):

```bash
python3 prueba_stream/servidor_mjpeg.py              # http://0.0.0.0:8080/video
python3 prueba_stream/servidor_mjpeg.py 8080 /dev/video0   # o tu webcam, para verla desde el teléfono
```

Fuente en la aplicación: `http://localhost:8080/video` en el mismo equipo, `http://10.0.2.2:8080/video` en el
emulador de Android (10.0.2.2 es el equipo anfitrión) o `http://IP-DEL-EQUIPO:8080/video` en un teléfono de la
misma red. El reloj del patrón sirve para comprobar que no se acumula retraso: si la aplicación no alcanza el
ritmo del stream, `QMediaPlayer` descarta cuadros y el reloj sigue al tiempo real.

Si la URL no responde, la línea de estado dice «Error del stream: …» y el botón vuelve a «Encender».

## Abrirlo

Qt Creator → *Open Project* → `CMakeLists.txt`, kits **Desktop Qt 6.12.0** y **Qt 6.12.0 for Android x86_64**
(requisitos en el [README de la serie](../README.md)). Necesita el módulo **Qt Multimedia** instalado en los dos
kits (Qt Maintenance Tool → Additional Libraries → Qt Multimedia).

## Cámara en el emulador de Android

El emulador puede usar la webcam de tu computadora como cámara trasera (`webcam0` es la primera webcam). Con el
emulador apagado:

```bash
sed -i 's/^hw.camera.back=.*/hw.camera.back=webcam0/' ~/.android/avd/Qt_Hola_API35.avd/config.ini
grep camera.back= ~/.android/avd/Qt_Hola_API35.avd/config.ini    # hw.camera.back=webcam0
```

y arráncalo de nuevo (`bash iniciar-emulador.sh`, o `--software` en una máquina virtual).

## Probado en

Ubuntu1 (VM), Qt 6.12.0, OpenCV 4.6.0 (escritorio, `libopencv-dev`) y OpenCV Android SDK 4.14.0 (emulador API 35,
x86_64). En la VM la webcam es virtual (`v4l2loopback` alimentada con `ffmpeg`). Stream de prueba de 15 cuadros/s:
unos 9.5 cuadros/s en el escritorio de la VM y de 0.3 a 1.4 en el emulador por software, sin retraso acumulado
(el reloj del patrón marcaba 45.2 s a los 45 s de conexión).

Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
