# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(invoke LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/invoke")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Scxml)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(invoke
    invoke.cpp
)

set_target_properties(invoke PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(invoke PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Scxml
)

# Statecharts:
qt6_add_statecharts(invoke
    statemachine.scxml
)

qt_add_qml_module(invoke
    URI InvokeExample
    VERSION 1.0
    QML_FILES
        Button.qml
        MainView.qml
        SubView.qml
    SOURCES
        invoke-qml.h
)

install(TARGETS invoke
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
