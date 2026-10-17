# Flutter y Dart 01: material para practicar

Este material acompaña el episodio de #tecnocircuito sobre cómo preparar Ubuntu 26.04 y VS Code para compilar una aplicación Flutter de escritorio para Linux.

**Enlace público del episodio:** https://github.com/esmarr58/tutoriales/tree/main/flutter-dart/01-entorno-ubuntu-2604

## Contenido

- [`comandos.md`](comandos.md): pasos de instalación completos, teclas para editar `~/.bashrc`, verificaciones, versiones y enlaces oficiales.
- `hola_flutter/`: proyecto inicial del contador, configuración para Linux y prueba automatizada.
- `hola_flutter/lib/main.dart`: código Dart de la aplicación.
- `hola_flutter/test/widget_test.dart`: prueba que verifica el incremento del contador.

El contador es la plantilla inicial generada por Flutter. En este episodio se prepara y comprueba el entorno; el código se estudiará por partes en los siguientes videos.

## Descarga y uso

Puedes abrir los archivos aquí mismo en GitHub. Para descargarlos, vuelve a la raíz del repositorio y pulsa **Code → Download ZIP**: se descargará el repositorio completo como `tutoriales-main.zip`. No necesitas crear una cuenta de GitHub.

Después de instalar Flutter, extrae el ZIP, abre una terminal en `hola_flutter` y ejecuta:

```bash
flutter pub get
flutter analyze
flutter test
flutter run -d linux
```

También puedes abrir VS Code con `code .`. No uses `sudo` para estos comandos. El SDK, las dependencias descargadas y las compilaciones se generan en tu equipo y no forman parte de esta descarga.

## Versiones comprobadas

- Ubuntu 26.04.1 LTS
- Flutter 3.47.7
- Dart 3.13.5
- VS Code 1.142.0

Si generas el proyecto con otra versión de Flutter, la plantilla puede variar. El análisis, la prueba, la compilación Linux y el incremento del contador se comprobaron en Ubuntu.

## Reto

Abre una terminal nueva, comprueba las versiones y ejecuta el proyecto. Desde una ejecución nueva, pulsa tres veces el botón más: el contador debe mostrar 3. Para salir de `flutter run`, vuelve a la terminal y pulsa `q`.
