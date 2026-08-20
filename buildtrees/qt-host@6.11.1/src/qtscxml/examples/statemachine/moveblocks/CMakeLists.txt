# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(moveblocks LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/animation/moveblocks")

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine Widgets)

qt_standard_project_setup()

qt_add_executable(moveblocks
    main.cpp
)

set_target_properties(moveblocks PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(moveblocks PRIVATE
    Qt::Core
    Qt::Gui
    Qt::StateMachine
    Qt::Widgets
)

install(TARGETS moveblocks
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
