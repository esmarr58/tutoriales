# Qt Android y SQLite #4 · Actualiza un producto al tocar su fila

Video: https://youtu.be/2RkwCFrql00

Proyecto `inventario_sqlite` (Qt Widgets, CMake) de la serie «Curso de Qt Creator · Qt Android y SQLite», Qt 6.12.0 en Ubuntu 24.04 con el kit Android x86_64 y el emulador `Qt_Hola_API35` (Android 15, API 35) del video [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs). También compila con el kit de escritorio.

Parte del final del video 3.

- `mainwindow.ui` — fila «Id:» con `etiquetaIdValor` al inicio del Form Layout y el botón `botonActualizar`.
- `mainwindow.h` — `int idSeleccionado = -1;` (sin fila elegida).
- `on_tablaProductos_cellClicked(int row, int column)` (Go to slot) — guarda el id de la columna 0 y llena el formulario.
- `on_botonActualizar_clicked()` (Go to slot) — `UPDATE productos SET nombre = :nombre, precio = :precio, existencia =
  :existencia WHERE id = :id`, `numRowsAffected()` y recarga.
- `variantes/reto_eliminar/` — solución del reto: botón Eliminar con `DELETE FROM productos WHERE id = :id`.

Prueba del video: tocar Lápiz → Id 2, precio 9 → «Filas actualizadas: 1.»; sin fila → «Primero toca un producto de la tabla.»

Letra: `centralwidget` con fuente de 20 pt (Property Editor → font → Point Size) para que el formulario se lea en el teléfono; todos los widgets la heredan.

## Abrir el ejemplo

En GitHub pulsa **Code → Download ZIP**, extrae `tutoriales-main.zip` y mueve `tutoriales-main` a `Proyectos` en tu carpeta personal. En Qt Creator elige **File → Open Project…**, abre `Proyectos/tutoriales-main/qt-creator/android-sqlite/04-actualizar-registros/inventario_sqlite/CMakeLists.txt`, marca el kit **Qt 6.12.0 for Android x86_64** (y Desktop si también quieres probarlo en el escritorio) y pulsa **Configure Project**. Arranca el emulador con `bash iniciar-emulador.sh --software` (carpeta [`../../android-sdk-hola-mundo`](../../android-sdk-hola-mundo/)), espera a que el selector Run device se vea en verde y ejecuta con Ctrl+R. Si no hiciste el video anterior, puedes empezar desde [`../03-consultar-tabla/inventario_sqlite`](../03-consultar-tabla/inventario_sqlite/).

El código de este ejemplo es el estado final del video: el formulario puede tener algún nombre o texto distinto de lo que se ve en cámara (por ejemplo, las etiquetas del Form Layout), sin efecto en el programa.

No incluye compilaciones, configuraciones personales ni archivos generados.
