cmake_minimum_required(VERSION 3.19)
project(camara_opencv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Multimedia trae QCamera, QMediaCaptureSession y QVideoSink.
# WebSockets (nuevo en el 08) trae QWebSocket: los comandos JSON al robot.
# 6.9: desde esa versión existe qt_add_android_permission (más abajo).
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Multimedia WebSockets)

qt_standard_project_setup()

# OpenCV: en el escritorio se usa el de Ubuntu (sudo apt install libopencv-dev).
# En Android se usa el OpenCV Android SDK descomprimido en ~/opencv.
if(ANDROID)
    set(OpenCV_DIR "$ENV{HOME}/opencv/OpenCV-android-sdk/sdk/native/jni")
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc)

qt_add_executable(camara_opencv
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    puente.cpp
    puente.h
    # Nuevo en el 08. Lógica pura, sin Qt ni OpenCV (se prueba sola: pruebas/prueba_logica.cpp):
    franja.cpp
    franja.h
    color.cpp
    color.h
    fsm.cpp
    fsm.h
    # Nuevo en el 08. Todo el OpenCV del programa:
    percepcion.cpp
    percepcion.h
)

target_link_libraries(camara_opencv
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
        Qt::WebSockets
        ${OpenCV_LIBS}
)

# Android: la aplicación declara que usará la cámara (va al AndroidManifest.xml).
# Además, el código pide el permiso al usuario la primera vez (QCameraPermission).
if(ANDROID)
    qt_add_android_permission(camara_opencv NAME android.permission.CAMERA)
    # Para la fuente por red (http://…) y para el WebSocket del robot (ws://…).
    qt_add_android_permission(camara_opencv NAME android.permission.INTERNET)
endif()

# Prueba de la lógica pura: un ejecutable de consola que no necesita Qt, OpenCV, cámara ni robot.
#   cmake --build . --target prueba_logica && ./prueba_logica ../pruebas/datos
if(NOT ANDROID)
    add_executable(prueba_logica
        pruebas/prueba_logica.cpp
        franja.cpp
        color.cpp
        fsm.cpp
    )
    target_include_directories(prueba_logica PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

include(GNUInstallDirs)

install(TARGETS camara_opencv
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET camara_opencv
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
