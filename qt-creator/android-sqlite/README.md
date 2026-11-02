# Qt Android y SQLite

Serie «Curso de Qt Creator · Qt Android y SQLite» (lista https://www.youtube.com/playlist?list=PLcSWCsj38VYM). Un mismo proyecto, `inventario_sqlite` (Qt Widgets + CMake, Qt 6.12.0, kit Android x86_64, emulador `Qt_Hola_API35`, Android 15), crece de un video al siguiente: cada carpeta es el estado final de su episodio.

Antes, prepara el SDK y el emulador con el video 0 [Qt Creator: de Ubuntu a Android](https://youtu.be/Jr2Uc-txARs) y los scripts de [`../android-sdk-hola-mundo`](../android-sdk-hola-mundo/).

- [Qt Android y SQLite #1 · Crea tu base de datos SQLite en el teléfono](01-crear-base/) · video https://youtu.be/3txcXlgBz_I
- [Qt Android y SQLite #2 · Guarda productos con INSERT, prepare y bindValue](02-insertar-registros/) · video https://youtu.be/dkPw2eT8x_s
- [Qt Android y SQLite #3 · Consulta la tabla y muéstrala en un QTableWidget](03-consultar-tabla/) · video https://youtu.be/9rzlx07t27Y
- [Qt Android y SQLite #4 · Actualiza un producto al tocar su fila](04-actualizar-registros/) · video https://youtu.be/2RkwCFrql00

La base `inventario.db` se crea en la carpeta privada de la aplicación en el teléfono (`QStandardPaths::AppDataLocation`); no viene en el repositorio.
