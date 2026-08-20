# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(scatter LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Quick Quick3D Graphs)

qt_standard_project_setup(REQUIRES 6.7)

qt_add_executable(scatter
    main.cpp
)

set_target_properties(scatter PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(scatter PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Quick3D
    Qt6::Graphs
)

qt_add_qml_module(scatter
    URI ScatterExample
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        "qml/scatter/Data.qml"
        "qml/scatter/Graph.qml"
        "qml/scatter/main.qml"
    RESOURCES
        "images/background.svg"
        "images/background_hide.svg"
        "images/camera.svg"
        "images/camera2.svg"
        "images/flatten.svg"
        "images/shadow.svg"
        "images/shadow_hide.svg"
        "images/smooth_curve.svg"
        "images/theme.svg"
)

install(TARGETS scatter
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
