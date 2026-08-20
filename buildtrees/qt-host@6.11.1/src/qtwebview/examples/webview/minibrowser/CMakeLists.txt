# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(minibrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webview/minibrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebView)

qt_add_executable(minibrowser
    main.cpp
)

set_target_properties(minibrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(minibrowser PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WebView
)

# Resources:
set(qml_resource_files
    "images/left-32.png"
    "images/refresh-32.png"
    "images/right-32.png"
    "images/stop-32.png"
    "images/settings-32.png"
    "main.qml"
)

qt6_add_resources(minibrowser "qml"
    PREFIX
        "/"
    BASE
        "images"
    FILES
        ${qml_resource_files}
)

install(TARGETS minibrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
