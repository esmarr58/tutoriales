# Qt Android y SQLite #2 · Guarda productos con INSERT, prepare y bindValue

Video: https://youtu.be/dkPw2eT8x_s

Proyecto `inventario_sqlite` (Qt Widgets, CMake) de la serie «Curso de Qt Creator · Qt Android y SQLite», Qt 6.12.0 en Ubuntu 24.04 con el kit Android x86_64 y el emulador `Qt_Hola_API35` (Android 15, API 35) del video [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs). También compila con el kit de escritorio.

Parte del final del video 1.

- `mainwindow.ui` — Form Layout con `entradaNombre` (QLineEdit), `spinPrecio` (QDoubleSpinBox, máximo 100000) y
  `spinExistencia` (QSpinBox, máximo 10000), y el botón `botonGuardar`.
- `on_botonGuardar_clicked()` (Go to slot) — valida el nombre, `prepare("INSERT INTO productos (nombre, precio, existencia)
  VALUES (:nombre, :precio, :existencia)")`, `bindValue`, `exec`, muestra `lastInsertId()` y actualiza el conteo.

Prueba del video: Cuaderno · 35 · 20 → «Guardado con id 1.»; ejercicio: Lápiz · 8.50 · 100 y Goma · 5 · 60 → ids 2 y 3.

Reto: no guardar con precio 0 («El precio debe ser mayor que cero.»).

Letra: `centralwidget` con fuente de 20 pt (Property Editor → font → Point Size) para que el formulario se lea en el teléfono; todos los widgets la heredan.

## Abrir el ejemplo

En GitHub pulsa **Code → Download ZIP**, extrae `tutoriales-main.zip` y mueve `tutoriales-main` a `Proyectos` en tu carpeta personal. En Qt Creator elige **File → Open Project…**, abre `Proyectos/tutoriales-main/qt-creator/android-sqlite/02-insertar-registros/inventario_sqlite/CMakeLists.txt`, marca el kit **Qt 6.12.0 for Android x86_64** (y Desktop si también quieres probarlo en el escritorio) y pulsa **Configure Project**. Arranca el emulador con `bash iniciar-emulador.sh --software` (carpeta [`../../android-sdk-hola-mundo`](../../android-sdk-hola-mundo/)), espera a que el selector Run device se vea en verde y ejecuta con Ctrl+R. Si no hiciste el video anterior, puedes empezar desde [`../01-crear-base/inventario_sqlite`](../01-crear-base/inventario_sqlite/).

El código de este ejemplo es el estado final del video: el formulario puede tener algún nombre o texto distinto de lo que se ve en cámara (por ejemplo, las etiquetas del Form Layout), sin efecto en el programa.

No incluye compilaciones, configuraciones personales ni archivos generados.
