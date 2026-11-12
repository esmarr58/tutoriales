cmake_minimum_required(VERSION 3.19)
project(camara_opencv LANGUAGES CXX)

# Multimedia trae QCamera, QMediaCaptureSession y QVideoSink.
# 6.9: desde esa versión existe qt_add_android_permission (más abajo).
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Multimedia)

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
)

target_link_libraries(camara_opencv
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
        ${OpenCV_LIBS}
)

# Android: la aplicación declara que usará la cámara (va al AndroidManifest.xml).
# Además, el código pide el permiso al usuario la primera vez (QCameraPermission).
if(ANDROID)
    qt_add_android_permission(camara_opencv NAME android.permission.CAMERA)
    # Para la fuente por red (http://…): el stream llega por Internet o por la red local.
    qt_add_android_permission(camara_opencv NAME android.permission.INTERNET)
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
