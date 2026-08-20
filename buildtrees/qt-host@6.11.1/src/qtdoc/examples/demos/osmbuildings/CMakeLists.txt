# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(OSMBuildings LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D Positioning)

qt_add_executable(osmbuildings
    main.cpp
)

if (ANDROID)
    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
    add_android_openssl_libraries(osmbuildings)
endif()

set_target_properties(osmbuildings PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(osmbuildings PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Quick3D
    Qt6::Positioning
)

qt_add_qml_module(osmbuildings
    URI Example
    QML_FILES Main.qml OSMCameraController.qml
    SOURCES request.h request.cpp
    SOURCES geometry.h geometry.cpp
    SOURCES manager.h manager.cpp
    SOURCES 3rdparty/mapbox/earcut.h
    RESOURCES customshaderbuildings.frag customshaderbuildings.vert customshadertiles.frag
    NO_RESOURCE_TARGET_PATH
    DEPENDENCIES
        QtQuick3D
)

install(TARGETS osmbuildings
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET osmbuildings
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
