# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(rogue LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/statemachine/rogue")

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine Widgets)

qt_standard_project_setup()

qt_add_executable(rogue
    main.cpp
    movementtransition.h
    window.cpp window.h
)

set_target_properties(rogue PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(rogue PRIVATE
    Qt::Core
    Qt::Gui
    Qt::StateMachine
    Qt::Widgets
)

install(TARGETS rogue
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
