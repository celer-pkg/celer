# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(trafficlight-widgets-static LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/trafficlight-widgets-static")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Scxml Widgets)

qt_standard_project_setup()

qt_add_executable(trafficlight-widgets-static
    ../trafficlight-common/trafficlight.cpp ../trafficlight-common/trafficlight.h
    trafficlight-widgets-static.cpp
)

set_target_properties(trafficlight-widgets-static PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(trafficlight-widgets-static PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Scxml
    Qt::Widgets
)

# Resources:
set_source_files_properties("../trafficlight-common/background.png"
    PROPERTIES QT_RESOURCE_ALIAS "background.png"
)

set_source_files_properties("../trafficlight-common/green.png"
    PROPERTIES QT_RESOURCE_ALIAS "green.png"
)

set_source_files_properties("../trafficlight-common/pause.png"
    PROPERTIES QT_RESOURCE_ALIAS "pause.png"
)

set_source_files_properties("../trafficlight-common/play.png"
    PROPERTIES QT_RESOURCE_ALIAS "play.png"
)

set_source_files_properties("../trafficlight-common/red.png"
    PROPERTIES QT_RESOURCE_ALIAS "red.png"
)

set_source_files_properties("../trafficlight-common/yellow.png"
    PROPERTIES QT_RESOURCE_ALIAS "yellow.png"
)

set(trafficlight-widgets-static_resource_files
    "../trafficlight-common/background.png"
    "../trafficlight-common/green.png"
    "../trafficlight-common/pause.png"
    "../trafficlight-common/play.png"
    "../trafficlight-common/red.png"
    "../trafficlight-common/yellow.png"
)

qt6_add_resources(trafficlight-widgets-static "trafficlight-widgets-static"
    PREFIX
        "/"
    FILES
        ${trafficlight-widgets-static_resource_files}
)

# Statecharts:
qt6_add_statecharts(trafficlight-widgets-static
    ../trafficlight-common/statemachine.scxml
)

install(TARGETS trafficlight-widgets-static
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
