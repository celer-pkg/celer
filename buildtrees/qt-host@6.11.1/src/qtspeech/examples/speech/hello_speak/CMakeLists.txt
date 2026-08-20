# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
project(hello_speak LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/speech/hello_speak")

find_package(Qt6 REQUIRED COMPONENTS Core Gui TextToSpeech Widgets)

qt_standard_project_setup()

qt_add_executable(hello_speak
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

set_target_properties(hello_speak PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(hello_speak PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::TextToSpeech
    Qt6::Widgets
)

install(TARGETS hello_speak
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
