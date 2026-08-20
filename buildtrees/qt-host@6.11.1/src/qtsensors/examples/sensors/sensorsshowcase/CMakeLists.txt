# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sensorsshowcase LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/sensors/sensorsshowcase")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Svg Sensors)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(sensorsshowcase
    main.cpp
)

set_target_properties(sensorsshowcase PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(ANDROID)
    set_property(TARGET sensorsshowcase PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()

if(APPLE AND IOS)
    set_property(TARGET sensorsshowcase PROPERTY
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
else()
    # default search path for the config file is "qrc:/"
    qt_add_resources(sensorsshowcase "resources"
        FILES qtquickcontrols2.conf)
endif()

qt_add_qml_module(sensorsshowcase
    URI SensorShowcaseModule
    VERSION 1.0
    QML_FILES
        "Main.qml"
        "Accelerometer.qml"
        "Compass.qml"
        "Gyroscope.qml"
        "Magnetometer.qml"
        "Proximity.qml"
        "ProgressXYZBar.qml"
    RESOURCES
        "images/compass.svg"
        "images/magnet.svg"
        "images/qt_logo.png"
    SOURCES
        sensorsupport.h
)

target_link_libraries(sensorsshowcase
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::Svg
        Qt::Sensors
)


install(TARGETS sensorsshowcase
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
