# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(trafficlight LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/statemachine/trafficlight")

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine Widgets)

qt_standard_project_setup()

qt_add_executable(trafficlight
    main.cpp
)

set_target_properties(trafficlight PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(trafficlight PRIVATE
    Qt::Core
    Qt::Gui
    Qt::StateMachine
    Qt::Widgets
)

install(TARGETS trafficlight
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
