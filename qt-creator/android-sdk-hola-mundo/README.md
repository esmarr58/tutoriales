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

Se supone que Qt Desktop ya funciona como en el curso básico. Abre `MaintenanceTool` de `~/Qt`, inicia sesión si la herramienta lo solicita y elige **Add or remove components**. Expande **Qt → Qt 6.12.0 → Android**. Instala Android **x86_64** para este emulador; ARM64 es la arquitectura típica de móviles modernos y se probó compilación, no ejecución física. Conserva Desktop gcc_64. Revisa los componentes y licencias, aplica los cambios y espera a que termine antes de abrir Creator.

## 3. Java, SDK, NDK e imagen

Desde la carpeta del material:

```bash
bash preparar-sdk.sh
```

Lee el script antes de ejecutarlo. Solicita sudo para apt; al escribir la contraseña no se muestran caracteres. Las licencias Android se presentan de forma interactiva; responde `y` solo si aceptas. No contiene aceptación automática ni cambia tu .bashrc.

Usa command-line tools **12.0** con ZIP y SHA-256 fijos del ensayo. También instala `latest`, pero los scripts llaman 12.0: latest 23.0 mostró aviso de migración a Android CLI y código 33 en este entorno. No se promete compatibilidad de estas versiones con futuros repositorios. Si falla una descarga o el hash, detente y revisa el error; no omitas la comprobación.

## 4. Configurar Qt Creator

Abre **Preferences → SDKs → Android** (en Creator 20). Configura:

- JDK location: `/usr/lib/jvm/java-21-openjdk-amd64`
- Android SDK location: tu carpeta personal, `Android/Sdk`; por ejemplo `/home/TU_USUARIO/Android/Sdk`.
- NDK: `27.2.12479018`, detectado dentro del SDK.
- Activa **Automatically create kits for Android tool chains**.
- Comprueba **Android settings are OK**. `SDK Version: 23.0` en ese mensaje se refiere a command-line tools, no a Android API 23.

Este ejemplo no usa HTTPS ni requiere OpenSSL. No confundas ese componente opcional con un error de JDK/SDK/NDK.

## 5. Aceleración y emulador

Android no admite el emulador acelerado dentro de otra VM. El ensayo con KVM anidado llegó a funcionar en Ubuntu1, pero al grabar ocurrió un reinicio inesperado de Windows cuya causa no quedó confirmada. **No recomendamos repetir KVM anidado.** El script ahora rechaza esa combinación. Para aceleración usa Ubuntu instalado directamente en el equipo. No basta con esperar si KVM no está disponible.

### Alternativa experimental para la grabación en una VM

```bash
bash iniciar-emulador.sh --software
```

Esta alternativa usa CPU por software (`-accel off`), GPU por software, 2 núcleos y 2048 MB. Puede tardar varios minutos y no sustituye la recomendación de Ubuntu físico. Sobre la imagen AOSP de depuración del episodio, el script reinicia ADB como root, fija `ro.hw_timeout_multiplier=10` y reinicia los servicios Android para ampliar sus esperas. No modifica los tiempos de espera de Windows ni reinicia la PC. La propiedad dura esta sesión del emulador. Espera a ver la pantalla de inicio; `adb devices` o `sys.boot_completed=1` por sí solos no prueban que la interfaz esté lista.

Los pasos de esta alternativa se ejecutaron individualmente durante la recuperación de la grabación: el APK se instaló desde Qt Creator y mostró Hola, Android. El arranque completo del nuevo script agrupado sobre un AVD recién creado queda pendiente de repetir. Se conservó el AVD original del ensayo y se utilizó una copia durante la recuperación. No se presenta como una prueba desde un sistema vacío.

### Registro del ensayo previo con virtualización anidada

Los pasos siguientes documentan el ensayo inicial; no son la ruta recomendada para una VM.

Para reproducir **Ubuntu1 en VirtualBox**: apaga Ubuntu normalmente (no guardar estado), conserva una instantánea y en Windows abre VirtualBox, selecciona la VM apagada, **Configuración → Sistema → Procesador → Habilitar VT-x/AMD-V anidado**. Si la opción no está disponible, revisa compatibilidad y configuración de virtualización del equipo; no fuerces pasos de este tutorial sobre una configuración diferente.

Alternativa exacta usada en el ensayo desde PowerShell, solo con esa VM apagada:

```powershell
& 'C:\Program Files\Oracle\VirtualBox\VBoxManage.exe' modifyvm Ubuntu1 --nested-hw-virt on
```

En Ubuntu:

```bash
sudo usermod -aG kvm "$USER"
```

Cierra sesión e inicia de nuevo para aplicar el grupo. Vuelve a abrir Terminal y la carpeta del material. Comprueba:

```bash
"$HOME/Android/Sdk/emulator/emulator" -accel-check
```

Debe indicar KVM instalado y usable. Si no existe el grupo kvm o /dev/kvm, detente: primero resuelve soporte/carga del módulo KVM; agregar un grupo no crea aceleración.

```bash
cd "$HOME/Proyectos/tutoriales-main/qt-creator/android-sdk-hola-mundo"
bash crear-emulador.sh
bash iniciar-emulador.sh
```

Mantén la terminal abierta. Sin opciones, el script usa GPU por software y CPU acelerada por KVM **solo fuera de una VM**. Dentro de VirtualBox se detiene y remite a la alternativa experimental anterior. Espera a la pantalla de inicio de Android; un marco de teléfono vacío no indica arranque completo. No inicies dos copias del mismo AVD.

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
- KVM no usable: resolver aceleración, no esperar indefinidamente.
- Mensaje de Java/NDK: comprobar rutas/versiones antes de modificar el proyecto.

Reto: cambia `text` de `etiquetaSaludo` a **Mi primera app Android**, guarda y ejecuta otra vez. Debe verse el texto nuevo dentro del emulador. El paquete generado es de desarrollo; no está preparado para distribución comercial ni firmado para Play Store.

## Verificación y alcance

Los comandos subyacentes se ejecutaron en Ubuntu1. Los tres scripts del material agrupan esos comandos con comprobaciones y licencias interactivas; no se ha repetido toda la descarga sobre una VM vacía. Se validó compilación x86_64/arm64, y ejecución x86_64 desde Qt Creator. Un celular comprado en 2025 no define su versión Android ni su arquitectura: prueba física pendiente de dispositivo concreto.

Referencias oficiales:

- https://doc.qt.io/qt-6/android-configure-dev-environment.html
- https://doc.qt.io/qtcreator/creator-how-to-set-android-preferences.html
- https://developer.android.com/studio/run/emulator-acceleration
