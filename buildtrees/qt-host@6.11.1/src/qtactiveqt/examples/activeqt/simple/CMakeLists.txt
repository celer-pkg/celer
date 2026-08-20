# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(simpleax LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/activeqt/simple")

find_package(Qt6 REQUIRED COMPONENTS AxServer Core Gui Widgets)
qt6_add_axserver_executable(simpleax
    main.cpp
    simple.def
    simple.rc
)

target_link_libraries(simpleax PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS simpleax
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
