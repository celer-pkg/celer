# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(colorpaletteApp LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Qml Quick OpenApi)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(colorpaletteApp
    main.cpp
)

add_subdirectory(QtOpenApiExampleStyle)

qt_add_qml_module(colorpaletteApp
    URI ColorPalette
    VERSION 1.0
    QML_FILES
        ColorPalette/Main.qml
        ColorPalette/ColorView.qml
        ColorPalette/UserMenu.qml
        ColorPalette/ColorDialogDelete.qml
        ColorPalette/ColorDialogEditor.qml
        ColorPalette/ServerSelection.qml
    SOURCES
        ApiBridges.h
    RESOURCES
        icons/delete.svg
        icons/delete_dark.svg
        icons/edit.svg
        icons/edit_dark.svg
        icons/plus.svg
        icons/plus_dark.svg
        icons/update.svg
        icons/update_dark.svg
        icons/userMask.svg
        icons/user.svg
        icons/user_dark.svg
        icons/qt.png
        icons/login.svg
        icons/login_dark.svg
        icons/logout.svg
        icons/logout_dark.svg
)

qt_add_library(ColorpaletteClientExample)

#! [0]
qt_add_openapi_client(ColorpaletteClientExample
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/colorpalette.yaml
)
#! [0]

target_link_libraries(colorpaletteApp PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    ColorpaletteClientExample
)

install(TARGETS colorpaletteApp
    BUNDLE  DESTINATION "${INSTALL_EXAMPLEDIR}"
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
