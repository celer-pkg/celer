# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(calculator LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/calculator")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Scxml)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(calculatorscxml
    calculator.cpp
)

set_target_properties(calculatorscxml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(calculatorscxml PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Scxml
)

# Statecharts:
qt6_add_statecharts(calculatorscxml
    statemachine.scxml
)

qt_add_qml_module(calculatorscxml
    URI Calculator
    VERSION 1.0
    QML_FILES
        MainWindow.qml
        Button.qml
    SOURCES
        calculator-qml.h
)

install(TARGETS calculatorscxml
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
