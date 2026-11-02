# Qt Android y SQLite #3 · Consulta la tabla y muéstrala en un QTableWidget

Video: https://youtu.be/9rzlx07t27Y

Proyecto `inventario_sqlite` (Qt Widgets, CMake) de la serie «Curso de Qt Creator · Qt Android y SQLite», Qt 6.12.0 en Ubuntu 24.04 con el kit Android x86_64 y el emulador `Qt_Hola_API35` (Android 15, API 35) del video [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs). También compila con el kit de escritorio.

Parte del final del video 2 (con Cuaderno, Lápiz y Goma guardados).

- `mainwindow.ui` — botón `botonCargar` y `tablaProductos` (QTableWidget de solo lectura: editTriggers sin opciones,
  selectionBehavior SelectRows; columnas Id, Nombre, Precio, Existencia).
- `on_botonCargar_clicked()` (Go to slot) — `SELECT id, nombre, precio, existencia FROM productos ORDER BY id`,
  `setRowCount(0)`, `while (next())` con `insertRow` y `setItem(… new QTableWidgetItem(value(columna).toString()))`.
- `on_botonGuardar_clicked()` recarga la tabla después de guardar.
- `variantes/reto_buscar/` — solución del reto: botón Buscar con `WHERE nombre LIKE :texto` y `%texto%` en `bindValue`
  («go» → solo Goma).

Letra: `centralwidget` con fuente de 20 pt (Property Editor → font → Point Size) para que el formulario se lea en el teléfono; todos los widgets la heredan.

## Abrir el ejemplo

En GitHub pulsa **Code → Download ZIP**, extrae `tutoriales-main.zip` y mueve `tutoriales-main` a `Proyectos` en tu carpeta personal. En Qt Creator elige **File → Open Project…**, abre `Proyectos/tutoriales-main/qt-creator/android-sqlite/03-consultar-tabla/inventario_sqlite/CMakeLists.txt`, marca el kit **Qt 6.12.0 for Android x86_64** (y Desktop si también quieres probarlo en el escritorio) y pulsa **Configure Project**. Arranca el emulador con `bash iniciar-emulador.sh --software` (carpeta [`../../android-sdk-hola-mundo`](../../android-sdk-hola-mundo/)), espera a que el selector Run device se vea en verde y ejecuta con Ctrl+R. Si no hiciste el video anterior, puedes empezar desde [`../02-insertar-registros/inventario_sqlite`](../02-insertar-registros/inventario_sqlite/).

El código de este ejemplo es el estado final del video: el formulario puede tener algún nombre o texto distinto de lo que se ve en cámara (por ejemplo, las etiquetas del Form Layout), sin efecto en el programa.

No incluye compilaciones, configuraciones personales ni archivos generados.
