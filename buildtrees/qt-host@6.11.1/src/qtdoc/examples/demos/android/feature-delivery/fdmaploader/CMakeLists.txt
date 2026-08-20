# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.30)

set(target_name fdmaploader)
set(target_uri io.qt.fdmaploader)

project(${target_name} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 6.10 REQUIRED COMPONENTS Quick)

set(featurelib_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/storeloader/")

qt_standard_project_setup(REQUIRES 6.10)

set(QT_USE_ANDROID_MODERN_BUNDLE ON)

qt6_add_executable(${target_name}
    main.cpp
    storeloader/playstoreloader.cpp storeloader/playstoreloader.h
)

qt6_add_qml_module(${target_name}
    URI ${target_uri}
    VERSION 1.0
    QML_FILES
        Main.qml BasicButton.qml
    RESOURCES
        images.qrc
    SOURCES
        maploader.h maploader.cpp
)

set_target_properties(${target_name} PROPERTIES
    QT_ANDROID_PACKAGE_NAME ${target_uri}
    QT_ANDROID_VERSION_CODE "1"
    QT_ANDROID_VERSION_NAME "0.1.1"
    QT_ANDROID_APP_ICON "@drawable/icon"
    QT_ANDROID_APP_NAME "Feature Delivery Map Loader"
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

qt_add_android_dynamic_feature_java_source_dirs(${target_name}
    SOURCE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/storeloader/java")

#! [Enable Feature Delivery]
qt6_add_android_dynamic_features(${target_name}
    FEATURE_TARGETS fdwintermapmodule)
#! [Enable Feature Delivery]

target_link_libraries(${target_name}
    PRIVATE Qt6::Quick
)

target_include_directories(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/storeloader/")

include(GNUInstallDirs)
install(TARGETS ${target_name}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

