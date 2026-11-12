# Qt y OpenCV #7 · Abre la cámara en escritorio y Android

Proyecto nuevo `camara_opencv`: enciende la cámara con Qt Multimedia y pasa cada cuadro a OpenCV.

```
QCamera → QMediaCaptureSession → QVideoSink → QVideoFrame → QImage → cv::Mat → (OpenCV) → QImage → QLabel
```

- **Encender / Apagar cámara**, **Cambiar cámara** (si hay más de una: frontal y trasera, o dos webcams).
- **Escala de grises (OpenCV)**: `cv::cvtColor(…, COLOR_BGR2GRAY)` sobre cada cuadro.
- Etiqueta de estado: nombre de la cámara, ancho × alto, canales y cuadros por segundo medidos.
- **Permiso de cámara**: en Android, `QCameraPermission` abre el diálogo del sistema la primera vez; en el
  escritorio ya está concedido. El `CMakeLists.txt` declara `android.permission.CAMERA` con
  `qt_add_android_permission`.
- **Descarte de cuadros**: se guarda solo el último cuadro y hay una sola conversión en espera. Sin esto, en un
  equipo lento los cuadros se acumulan, la memoria crece y Android cierra la aplicación.

**Prueba esperada.** Escritorio (Ubuntu): «<nombre de tu webcam> · 640 × 480 · 3 canales · ~15–30 cuadros/s»; con
la casilla, «1 canal» y la imagen en gris. Android: diálogo de permiso, «Mientras se usa la app», y la imagen de la
cámara trasera en el teléfono, girada en vertical (480 × 640). Al apagar, queda el último cuadro y la casilla
lo sigue procesando.

En el emulador por software (sin aceleración, como en una VM) llegan de 0.4 a 1.5 cuadros por segundo: es el
emulador, no el programa: en el escritorio, el mismo programa muestra 15 cuadros/s, los que manda la webcam virtual.

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
x86_64). En la VM la webcam es virtual (`v4l2loopback` alimentada con `ffmpeg`).

Qué cambió respecto al episodio anterior: [CAMBIOS.md](CAMBIOS.md).
