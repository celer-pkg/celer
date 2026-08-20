# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(trafficlight-qml-dynamic LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/trafficlight-qml-dynamic")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Scxml)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(trafficlight-qml-dynamic
    trafficlight-qml-dynamic.cpp
)

set_target_properties(trafficlight-qml-dynamic PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(trafficlight-qml-dynamic PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Scxml
)

set_source_files_properties("../trafficlight-common/statemachine.scxml"
    PROPERTIES QT_RESOURCE_ALIAS "statemachine.scxml"
)

set_source_files_properties(../trafficlight-common/play.png
    PROPERTIES QT_RESOURCE_ALIAS "play.png"
)

set_source_files_properties(../trafficlight-common/play.png
    PROPERTIES QT_RESOURCE_ALIAS "play.png"
)

set_source_files_properties(../trafficlight-common/yellow.png
    PROPERTIES QT_RESOURCE_ALIAS "yellow.png"
)

set_source_files_properties(../trafficlight-common/red.png
    PROPERTIES QT_RESOURCE_ALIAS "red.png"
)

set_source_files_properties(../trafficlight-common/green.png
    PROPERTIES QT_RESOURCE_ALIAS "green.png"
)

set_source_files_properties(../trafficlight-common/background.png
    PROPERTIES QT_RESOURCE_ALIAS "background.png"
)

set_source_files_properties(../trafficlight-common/pause.png
    PROPERTIES QT_RESOURCE_ALIAS "pause.png"
)

qt_add_qml_module(trafficlight-qml-dynamic
    URI TrafficLightApplication
    VERSION 1.0
    QML_FILES
        MainView.qml
        Button.qml
        Lights.ui.qml
    RESOURCES
        ../trafficlight-common/statemachine.scxml
        ../trafficlight-common/play.png
        ../trafficlight-common/yellow.png
        ../trafficlight-common/red.png
        ../trafficlight-common/green.png
        ../trafficlight-common/background.png
        ../trafficlight-common/pause.png
)

install(TARGETS trafficlight-qml-dynamic
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
