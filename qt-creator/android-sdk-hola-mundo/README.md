# Qt Widgets para Android desde Ubuntu

Objetivo: preparar Qt Creator, construir un APK y ejecutar **Hola, Android** en un emulador Android 15. No es un procedimiento para publicar en Play Store ni una prueba en un teléfono físico.

## Entorno comprobado

- Ubuntu **24.04.5 x86_64** en VirtualBox (VM Ubuntu1), 8 GB RAM y 4 CPU asignados. Este tutorial no usa las VM Ubuntu 26 de Flutter.
- Qt **6.12.0** Desktop y Android; Qt Creator **20.0.2** instalados con el instalador de Qt en `~/Qt`.
- JDK 21, NDK **27.2.12479018**, compile/target SDK **36**, build-tools **36.0.0**.
- Imagen Android **15 / API 35 / x86_64**, AVD **Qt_Hola_API35**, perfil Pixel 5. El perfil no significa que se probó un teléfono físico de ese modelo.
- SDK 37.0/build-tools 37.0.0 también instalados porque la metadata de Qt de este ensayo los solicitó. El ejemplo se configura explícitamente con 36.
- La imagen y el SDK descargan varios GB. Necesitas Internet, espacio libre suficiente y acceso sudo.

## 1. Material y carpeta de trabajo

En este repositorio pulsa **Code → Download ZIP**. En Ubuntu abre **Archivos → Descargas**, extrae `tutoriales-main.zip`. Crea `Proyectos` en tu carpeta personal si falta y mueve allí `tutoriales-main`. Abre:

`~/Proyectos/tutoriales-main/qt-creator/android-sdk-hola-mundo`

Si tu administrador de archivos ofrece Abrir en terminal, úsalo aquí. También puedes abrir Terminal con Ctrl+Alt+T y pegar:

```bash
cd "$HOME/Proyectos/tutoriales-main/qt-creator/android-sdk-hola-mundo"
pwd
ls
```

Debes ver `preparar-sdk.sh`, `crear-emulador.sh`, `iniciar-emulador.sh` y `hola_android`. Si no aparecen, corrige la carpeta antes de continuar. Si extrajiste el ZIP con otro nombre, ajusta la ruta. Los scripts se abren con editor de texto para revisarlos y se ejecutan con **bash**, no requieren chmod.

## 2. Componentes de Qt

Se supone que Qt Desktop ya funciona como en el curso básico. Abre `MaintenanceTool` de `~/Qt`, inicia sesión si la herramienta lo solicita y elige **Add or remove components**. Expande **Qt 6.12.0** y marca **Android**. En la versión mostrada, Android es un componente agrupado que incluye varias arquitecturas: no hay que desplegarlo para elegir x86_64. Esa arquitectura se selecciona después en **Configure Project**. Conserva Desktop gcc_64 y pulsa **Next** para revisar los cambios. Si Android ya está instalado, consérvalo; no lo desmarques para reinstalarlo. Si lo estás añadiendo, revisa licencias, confirma y espera a que termine. Cierra MaintenanceTool antes de abrir Creator.

## 3. Java, SDK, NDK e imagen

Desde la carpeta del material:

```bash
bash preparar-sdk.sh
```

Lee el script antes de ejecutarlo. Solicita sudo para apt; al escribir la contraseña no se muestran caracteres. Las licencias Android se presentan de forma interactiva; responde `y` solo si aceptas. No contiene aceptación automática ni cambia tu .bashrc.

Usa command-line tools **12.0** con ZIP y SHA-256 fijos del ensayo. También instala `latest`, pero los scripts llaman 12.0: latest 23.0 mostró aviso de migración a Android CLI y código 33 en este entorno. No se promete compatibilidad de estas versiones con futuros repositorios. Si falla una descarga o el hash, detente y revisa el error; no omitas la comprobación.

## 4. Configurar Qt Creator

Abre **Preferences → SDKs → Android** (en Creator 20). Configura:

- JDK location: pulsa **Browse**, selecciona `/usr/lib/jvm/java-21-openjdk-amd64` y confirma con **Choose**. Comprueba que el campo conserva la ruta completa, no solo `/`.
- Android SDK location: pulsa **Browse**, abre tu carpeta personal, **Android → Sdk** y confirma con **Choose**. La ruta queda dentro de tu carpeta personal; por ejemplo `/home/TU_USUARIO/Android/Sdk`. En la grabación aparece `/home/r/Android/Sdk` porque el usuario del equipo se llama `r`. No crees una carpeta `r` ni copies ese nombre si tu usuario es diferente. **Android** y **.android** son carpetas distintas; el SDK está en la primera.
- NDK: `27.2.12479018`, detectado dentro del SDK.
- Activa **Automatically create kits for Android tool chains**.
- Comprueba **Android settings are OK**. `SDK Version: 23.0` en ese mensaje se refiere a command-line tools, no a Android API 23.

La página Android de Creator 20 indica que sus ajustes tienen efecto inmediato. Antes de salir comprueba que las dos rutas siguen completas, el NDK aparece y Android settings are OK. Este ejemplo no usa HTTPS ni requiere OpenSSL. No confundas ese componente opcional con un error de JDK/SDK/NDK.

## 5. Crear y arrancar el emulador por software

Este es el recorrido de la grabación dentro de VirtualBox: **sin KVM ni virtualización anidada**. No apagues Ubuntu ni cambies las opciones de VirtualBox para estos pasos. La alternativa por software es lenta y experimental; no promete rendimiento adecuado para cualquier equipo. Para aceleración, consulta la documentación de Android y usa un entorno físico compatible, fuera de otra VM. El script rechaza KVM anidado; no mezcles las dos rutas.

En la terminal de la carpeta del material:

```bash
cd "$HOME/Proyectos/tutoriales-main/qt-creator/android-sdk-hola-mundo"
bash crear-emulador.sh
bash iniciar-emulador.sh --software
```

El primer script crea **Qt_Hola_API35** o conserva el existente. La opción **--software** del segundo comando es necesaria para este recorrido. No omitas los dos guiones ni sustituyas el comando por otra variante. Mantén la terminal abierta. Si ya hay un emulador en emulator-5554, el script pide cerrarlo antes de iniciar otro; no lances dos copias.

Usa CPU por software (`-accel off`), GPU por software, 2 núcleos y 2048 MB. Sobre la imagen AOSP de depuración del episodio, el script reinicia ADB como root, fija `ro.hw_timeout_multiplier=10` y reinicia los servicios Android. Esto amplía las esperas dentro del Android emulado; no modifica Windows ni reinicia la PC. La propiedad pertenece a esa sesión.

El primer arranque tarda varios minutos. Espera la **pantalla de inicio de Android**, con sus iconos, no solo el marco del teléfono. Si queda negra, pulsa el **botón circular Inicio** del emulador. En la grabación se recorta la espera.

Abre otra terminal con **Ctrl+Alt+T** y ejecuta:

```bash
"$HOME/Android/Sdk/platform-tools/adb" devices
```

Debe aparecer `emulator-5554    device`. Si figura `offline`, espera. `device` y `sys.boot_completed=1` no sustituyen la comprobación visual del escritorio.

El 2026-10-18 se repitió el script agrupado `--software` desde un **AVD nuevo y aislado**, conservando el anterior. Se comprobó `sys.boot_completed=1`, la propiedad de esperas en 10, ADB en estado device y el escritorio tras pulsar Inicio. Esto valida ese arranque concreto, no toda la instalación desde una VM vacía ni el rendimiento en otros equipos.

## 6. Abrir y ejecutar el proyecto

En Qt Creator: **File → Open File or Project**, selecciona `hola_android/CMakeLists.txt` dentro del material. Marca kit **Qt 6.12.0 for Android x86_64** y **Configure Project**. No elijas Desktop ni ARM64 para este emulador x86_64.

En **Projects → Build Settings → Build Steps → Build Android APK → Details**, selecciona:

- Android build-tools version: **36.0.0**.
- Android build platform SDK: **android-36**.

El proyecto ya contiene una ventana Qt Widgets con una etiqueta centrada. En `mainwindow.ui`, Object Inspector permite seleccionar `etiquetaSaludo`; su propiedad `text` muestra `Hola, Android`. El código se entrega completo para concentrar el episodio en el entorno, no en aprender un segundo diseñador.

Pulsa **Ctrl+B** y comprueba compilación correcta. Pulsa **Ctrl+R**; selecciona el emulador **Qt_Hola_API35 / emulator-5554** si solicita dispositivo. Debe instalar la app y mostrar **Hola, Android**. Que exista un APK no demuestra que la app haya arrancado.

## Diagnóstico y reto

- No hay kit Android: revisar componente Android de Qt y creación automática de kits.
- No hay dispositivo: encender AVD, esperar arranque y comprobar `~/Android/Sdk/platform-tools/adb devices`; debe figurar `device`, no `offline`.
- El script pide KVM: verifica que escribiste `--software` como en este recorrido; no actives virtualización anidada para continuarlo.
- Mensaje de Java/NDK: comprobar rutas/versiones antes de modificar el proyecto.

Reto: cambia `text` de `etiquetaSaludo` a **Mi primera app Android**, guarda y ejecuta otra vez. Debe verse el texto nuevo dentro del emulador. El paquete generado es de desarrollo; no está preparado para distribución comercial ni firmado para Play Store.

## Verificación y alcance

Los comandos subyacentes se ejecutaron en Ubuntu1. Los tres scripts del material agrupan esos comandos con comprobaciones y licencias interactivas; no se ha repetido toda la descarga sobre una VM vacía. Se validó compilación x86_64/arm64, y ejecución x86_64 desde Qt Creator. Un celular comprado en 2025 no define su versión Android ni su arquitectura: prueba física pendiente de dispositivo concreto.

Referencias oficiales:

- https://doc.qt.io/qt-6/android-configure-dev-environment.html
- https://doc.qt.io/qtcreator/creator-how-to-set-android-preferences.html
- https://developer.android.com/studio/run/emulator-acceleration
