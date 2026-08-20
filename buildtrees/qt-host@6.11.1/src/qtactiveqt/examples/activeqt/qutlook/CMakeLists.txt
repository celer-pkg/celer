# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qutlook LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/activeqt/qutlook")

find_package(Qt6 REQUIRED COMPONENTS AxContainer Core Gui Widgets)

qt_add_executable(qutlook
    addressview.cpp addressview.h
    main.cpp
)

set_target_properties(qutlook PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

qt6_target_typelibs(qutlook LIBRARIES "msoutl:{00062FFF-0000-0000-C000-000000000046}")

target_link_libraries(qutlook PUBLIC
    Qt::AxContainer
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS qutlook
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
