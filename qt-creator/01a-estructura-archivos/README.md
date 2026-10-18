# Qt Creator 01.a — estructura de los archivos

Complemento entre los episodios 01 y 02. Requiere Qt 6 Desktop, Qt Creator y el kit C++ del episodio 01. Ejemplo probado con Qt 6.12.0 en Ubuntu 24.04.

## Abrir el ejemplo

En GitHub pulsa **Code → Download ZIP**. En Archivos abre Descargas, extrae `tutoriales-main.zip` y mueve `tutoriales-main` a `Proyectos` dentro de tu carpeta personal (crea Proyectos si falta). En Qt Creator elige **File → Open File or Project**, abre `Proyectos/tutoriales-main/qt-creator/01a-estructura-archivos/hola_mundo/CMakeLists.txt`, selecciona tu kit Desktop y confirma Configure Project. Ejecuta con el triángulo verde.

También puedes seguir con el proyecto que creaste en el episodio 01. No necesitas reemplazarlo para este ejercicio.

| Archivo | Para qué sirve en este ejemplo |
|---|---|
| main.cpp | Punto de entrada, creación de ventana y eventos |
| mainwindow.h | Declaración de la clase y sus funciones |
| mainwindow.cpp | Implementación; incluye la acción del botón |
| mainwindow.ui | Formulario y texto inicial |
| CMakeLists.txt | Configuración para construir el programa |

`ui_mainwindow.h` es generado; no se edita manualmente. Los grupos Sources, Headers y Forms de Qt Creator pueden ser categorías de la vista, no carpetas reales. La ubicación del directorio de compilación depende de tu kit y configuración.

## Ejercicio

En `mainwindow.cpp`, dentro de `on_botonSaludar_clicked`, cambia únicamente el texto entre comillas:

```cpp
ui->etiquetaMensaje->setText("Hola, equipo");
```

Guarda, cierra la aplicación anterior y vuelve a ejecutar. Al abrir debe mostrar **Presiona el botón**; al pulsar **Saludar**, **Hola, equipo**.

## Reto y comprobación

Haz que al abrir muestre **Listo para practicar** y al pulsar **Prueba completada**. Al cerrar y abrir otra vez debe volver al mensaje inicial.

Pista: en `mainwindow.ui`, selecciona `etiquetaMensaje` en Object Inspector y cambia la propiedad `text`. El texto después del clic se cambia en la función de `mainwindow.cpp`.

No incluye compilaciones, configuraciones personales ni archivos generados. Fuente de referencia: https://doc.qt.io/qt-6/cmake-get-started.html
