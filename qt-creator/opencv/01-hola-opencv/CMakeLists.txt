cmake_minimum_required(VERSION 3.19)
project(visor_opencv LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# OpenCV: en el escritorio se usa el de Ubuntu (sudo apt install libopencv-dev).
# En Android se usa el OpenCV Android SDK descomprimido en ~/opencv.
# Su OpenCVConfig.cmake elige la carpeta de la ABI del kit (x86_64 en el emulador)
# y enlaza las bibliotecas estáticas: solo entra al APK lo que usamos.
if(ANDROID)
    set(OpenCV_DIR "$ENV{HOME}/opencv/OpenCV-android-sdk/sdk/native/jni")
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc)

qt_add_executable(visor_opencv
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(visor_opencv
    PRIVATE
        Qt::Core
        Qt::Widgets
        ${OpenCV_LIBS}
)

install(TARGETS visor_opencv
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET visor_opencv
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
