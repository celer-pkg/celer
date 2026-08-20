# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(compacthealth LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui CanvasPainter OPTIONAL_COMPONENTS GuiPrivate)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(compacthealthexample
    WIN32
    MACOSX_BUNDLE
    main.cpp
    painterwindow.cpp painterwindow.h
    mainwindow.cpp mainwindow.h
    ecggraph.cpp ecggraph.h
    theme.h theme.cpp
)

target_link_libraries(compacthealthexample PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::CanvasPainter
)

qt_add_resources(compacthealthexample "compacthealthexample"
    PREFIX
        /
    FILES
        images/icon_random_dark.png
        images/icon_random_light.png
        images/icon_theme_dark.png
        images/icon_theme_light.png
        images/icon_run_dark.png
        images/icon_run_light.png
        images/icon_settings_dark.png
        images/icon_settings_light.png
)

install(TARGETS compacthealthexample
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET compacthealthexample
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
