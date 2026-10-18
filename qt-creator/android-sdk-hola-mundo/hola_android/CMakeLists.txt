cmake_minimum_required(VERSION 3.21)
project(HolaAndroid VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()
qt_add_executable(HolaAndroid main.cpp mainwindow.cpp mainwindow.h mainwindow.ui)
target_link_libraries(HolaAndroid PRIVATE Qt6::Widgets)
set_target_properties(HolaAndroid PROPERTIES
    QT_ANDROID_PACKAGE_NAME "app.bitgatelab.holaandroid"
    QT_ANDROID_VERSION_NAME "0.1"
    QT_ANDROID_VERSION_CODE 1
    QT_ANDROID_COMPILE_SDK_VERSION 36
    QT_ANDROID_TARGET_SDK_VERSION 36
    QT_ANDROID_SDK_BUILD_TOOLS_REVISION "36.0.0"
)
