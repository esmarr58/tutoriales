# Qt Android y SQLite #1 · Crea tu base de datos SQLite en el teléfono

Video: https://youtu.be/3txcXlgBz_I

Proyecto `inventario_sqlite` (Qt Widgets, CMake) de la serie «Curso de Qt Creator · Qt Android y SQLite», Qt 6.12.0 en Ubuntu 24.04 con el kit Android x86_64 y el emulador `Qt_Hola_API35` (Android 15, API 35) del video [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs). También compila con el kit de escritorio.

- `inventario_sqlite/CMakeLists.txt` — `Sql` en `find_package` y `Qt::Sql` en `target_link_libraries`.
- `mainwindow.cpp` — el constructor abre `inventario.db` con el driver `QSQLITE` en
  `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` (la carpeta privada de la aplicación), la crea con
  `QDir().mkpath()` si falta y ejecuta `CREATE TABLE IF NOT EXISTS productos (…)`. `etiquetaEstado` dice «Base nueva creada» la
  primera vez y «Base encontrada» las siguientes.
- `on_botonComprobar_clicked()` (creado con Go to slot) cuenta las filas con `SELECT COUNT(*) FROM productos`.
- `variantes/sin_mkpath/` — sin `mkpath`: en el escritorio, si la carpeta no existe, `open()` falla con «unable to open
  database file».

Prueba del video: primera ejecución «Base nueva creada: …/inventario.db» y «Productos en la tabla: 0»; cerrar y abrir →
«Base encontrada».

Reto: una segunda tabla `categorias (id, nombre)` y su conteo en Comprobar base, sin error al abrir dos veces.

Letra: `centralwidget` con fuente de 20 pt (Property Editor → font → Point Size) para que el formulario se lea en el teléfono; todos los widgets la heredan.

## Abrir el ejemplo

En GitHub pulsa **Code → Download ZIP**, extrae `tutoriales-main.zip` y mueve `tutoriales-main` a `Proyectos` en tu carpeta personal. En Qt Creator elige **File → Open Project…**, abre `Proyectos/tutoriales-main/qt-creator/android-sqlite/01-crear-base/inventario_sqlite/CMakeLists.txt`, marca el kit **Qt 6.12.0 for Android x86_64** (y Desktop si también quieres probarlo en el escritorio) y pulsa **Configure Project**. Arranca el emulador con `bash iniciar-emulador.sh --software` (carpeta [`../../android-sdk-hola-mundo`](../../android-sdk-hola-mundo/)), espera a que el selector Run device se vea en verde y ejecuta con Ctrl+R.

El código de este ejemplo es el estado final del video: el formulario puede tener algún nombre o texto distinto de lo que se ve en cámara (por ejemplo, las etiquetas del Form Layout), sin efecto en el programa.

No incluye compilaciones, configuraciones personales ni archivos generados.
