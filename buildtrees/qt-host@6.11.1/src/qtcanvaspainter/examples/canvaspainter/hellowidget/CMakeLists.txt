# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(hellowidget LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets CanvasPainter)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(canvaspainterhellowidget
    WIN32
    MACOSX_BUNDLE
    main.cpp
    canvaswidget.cpp canvaswidget.h
)

target_link_libraries(canvaspainterhellowidget PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::CanvasPainter
)

qt_add_resources(canvaspainterhellowidget "canvaspainterhellowidget"
    PREFIX
        /
    FILES
        qt-translucent.png
)

install(TARGETS canvaspainterhellowidget
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET canvaspainterhellowidget
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
