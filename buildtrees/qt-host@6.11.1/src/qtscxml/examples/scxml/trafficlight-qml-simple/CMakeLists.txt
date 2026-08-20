# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(trafficlight-qml-simple LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/trafficlight-qml-simple")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Scxml)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(trafficlight-qml-simple
    trafficlight-qml-simple.cpp
)

set_target_properties(trafficlight-qml-simple PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(trafficlight-qml-simple PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Scxml
)

# Statecharts:
qt6_add_statecharts(trafficlight-qml-simple
    ../trafficlight-common/statemachine.scxml
)

qt_add_qml_module(trafficlight-qml-simple
    URI TrafficLightApplication
    VERSION 1.0
    QML_FILES
        MainView.qml
        Light.qml
    SOURCES
        trafficlight-qml.h
)

install(TARGETS trafficlight-qml-simple
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
