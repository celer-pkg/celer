# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(cockpit LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Graphs)
find_package(Qt6 COMPONENTS ShaderTools)

qt_add_executable(cockpit
    main.cpp
)
set_target_properties(cockpit PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(cockpit PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Graphs
    Qt::ShaderTools
)

qt6_add_qml_module(cockpit
    URI CockpitExample
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        qml/cockpit/main.qml
        qml/cockpit/Speedometer.qml
        qml/cockpit/Map.qml
        qml/cockpit/ArtificialHorizon.qml
        qml/cockpit/VibrationSensor.qml
        qml/cockpit/ControlPanel.qml
)

qt6_add_shaders(cockpit "cockpit_shaders"
    GLSL "300es, 330"
    PREFIX
        ""
    FILES
        qml/cockpit/bar.frag
        qml/cockpit/circleMarker.frag
        qml/cockpit/pitchbar.frag
)

qt6_add_resources(cockpit "cockpit_resources"
    PREFIX
        ""
    FILES
        qml/cockpit/airplane-ico.png
        qml/cockpit/plane-fro.png
)

install(TARGETS cockpit
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
