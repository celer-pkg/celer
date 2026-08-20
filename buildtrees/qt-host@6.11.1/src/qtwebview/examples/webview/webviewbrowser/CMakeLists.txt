# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(webviewbrowser LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebView)
qt_standard_project_setup()

qt_add_executable(webviewbrowser
    browserwindow.cpp browserwindow.h
    main.cpp
)

set_target_properties(webviewbrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(webviewbrowser PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::WebView
)

# Resources:
set(images_resource_files
    "images/left-32.png"
    "images/refresh-32.png"
    "images/right-32.png"
    "images/stop-32.png"
    "images/settings-32.png"
)

qt_add_resources(webviewbrowser "images"
    PREFIX
        "/"
    BASE
        "images"
    FILES
        ${images_resource_files}
)

install(TARGETS webviewbrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET webviewbrowser
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
