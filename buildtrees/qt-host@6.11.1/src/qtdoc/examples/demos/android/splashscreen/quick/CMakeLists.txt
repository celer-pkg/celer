cmake_minimum_required(VERSION 3.22)

set(target_name qmlsplashscreeninandroid)
set(app_target_name app${target_name})

project(${target_name} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(${app_target_name}
    main.cpp
)

qt_add_qml_module(${app_target_name}
    URI ${target_name}
    QML_FILES
        Main.qml
        Splash.qml
    RESOURCES
        qt_logo.png
)

target_link_libraries(${app_target_name}
    PRIVATE Qt6::Quick
)

if(ANDROID)
    set_target_properties(${app_target_name} PROPERTIES
        QT_ANDROID_PACKAGE_NAME "io.qt.qmlsplashscreeninandroid"
        QT_ANDROID_VERSION_CODE "100"
        QT_ANDROID_VERSION_NAME "0.1.0"
        QT_ANDROID_APP_ICON "@mipmap/icon"
        QT_ANDROID_APP_NAME "QML Splash Screen in Android"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
endif()

include(GNUInstallDirs)
install(TARGETS ${app_target_name}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
