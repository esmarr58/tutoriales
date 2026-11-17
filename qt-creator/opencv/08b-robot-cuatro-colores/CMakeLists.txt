cmake_minimum_required(VERSION 3.19)
project(robot_colores LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Multimedia trae QCamera, QMediaCaptureSession y QVideoSink; WebSockets, QWebSocket.
# 6.9: desde esa versión existe qt_add_android_permission (más abajo).
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Multimedia WebSockets)

qt_standard_project_setup()

# OpenCV: en el escritorio se usa el de Ubuntu (sudo apt install libopencv-dev).
# En Android se usa el OpenCV Android SDK descomprimido en ~/opencv.
if(ANDROID)
    set(OpenCV_DIR "$ENV{HOME}/opencv/OpenCV-android-sdk/sdk/native/jni")
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc)

qt_add_executable(robot_colores
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    puente.cpp
    puente.h
    # Lógica pura, sin Qt ni OpenCV (se prueba sola: pruebas/prueba_logica.cpp):
    color.cpp
    color.h
    fsm_p5.cpp
    fsm_p5.h
    # Todo el OpenCV del programa:
    zona.cpp
    zona.h
)

target_link_libraries(robot_colores
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
        Qt::WebSockets
        ${OpenCV_LIBS}
)

# Android: la aplicación declara que usará la cámara y la red (va al AndroidManifest.xml).
if(ANDROID)
    qt_add_android_permission(robot_colores NAME android.permission.CAMERA)
    qt_add_android_permission(robot_colores NAME android.permission.INTERNET)
endif()

# Prueba de la lógica pura: un ejecutable de consola que no necesita Qt, OpenCV, cámara ni robot.
if(NOT ANDROID)
    add_executable(prueba_logica
        pruebas/prueba_logica.cpp
        color.cpp
        fsm_p5.cpp
    )
    target_include_directories(prueba_logica PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

include(GNUInstallDirs)

install(TARGETS robot_colores
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET robot_colores
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
