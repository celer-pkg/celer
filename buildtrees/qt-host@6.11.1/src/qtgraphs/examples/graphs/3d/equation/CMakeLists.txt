# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(equation LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick Quick3D Graphs)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(equation
    main.cpp
)
set_target_properties(equation PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(equation PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Quick3D
    Qt6::Graphs
)

qt_add_qml_module(equation
    URI EquationExample
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        "qml/equation/Main.qml"
    RESOURCES
        "images/opacitymap.png"
        "qml/equation/calculator.js"
)

install(TARGETS equation
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
