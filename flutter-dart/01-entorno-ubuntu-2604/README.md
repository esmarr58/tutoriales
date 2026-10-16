# Flutter y Dart 01 ? Material para practicar

Este paquete acompa?a el episodio de instalación del entorno Flutter para escritorio Linux en Ubuntu 26.04 con VS Code, del canal #tecnocircuito.

## Qué contiene

- `comandos.md`: instalación paso a paso, comprobaciones, versiones y referencias.
- `hola_flutter/`: proyecto del contador, código Dart, configuración Linux y prueba automatizada.
- `hola_flutter/lib/main.dart`: entrada y pantalla del contador.
- `hola_flutter/test/widget_test.dart`: prueba del incremento del contador.

El contador es el ejemplo inicial generado por Flutter, sin modificaciones didácticas en este episodio. Aquí comprobamos la instalación; su código se estudiará por partes en los siguientes videos.

## Cómo utilizarlo

Primero sigue `comandos.md` para instalar el entorno en Ubuntu. Después extrae este ZIP en una carpeta de tu usuario y abre una terminal dentro de `hola_flutter`:

```bash
flutter pub get
flutter analyze
flutter test
flutter run -d linux
```

No uses sudo para ejecutar estos comandos. Para abrir el proyecto en VS Code, ejecuta `code .` desde esa misma carpeta. Las dependencias y archivos generados se reconstruyen en tu equipo; no se incluyen compilaciones ni cachés.

Puedes seguir el video creando el proyecto desde cero, o abrir esta copia para comparar. Si creas el proyecto con otra versión de Flutter, la plantilla inicial puede variar. Versiones usadas: Flutter 3.47.7 y Dart 3.13.5. Las comprobaciones de análisis, prueba y ejecución del episodio se realizaron en Ubuntu.

## Reto y resultado esperado

1. Abre una terminal nueva y comprueba `flutter --version` y `dart --version`.
2. En la carpeta del proyecto, ejecuta la app Linux.
3. Desde una ejecución nueva, pulsa tres veces el botón más: el contador debe mostrar 3.
4. Vuelve a la terminal de ejecución y pulsa `q` para salir.

La prueba incluida comprueba el paso de 0 a 1. El reto de tres pulsaciones se comprueba manualmente.


## Acceso al material

Carpeta del episodio: https://github.com/esmarr58/tutoriales/tree/main/flutter-dart/01-entorno-ubuntu-2604

Descarga del repositorio: https://github.com/esmarr58/tutoriales/archive/refs/heads/main.zip
