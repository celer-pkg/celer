# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(logfilepositionsource LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/positioning/logfilepositionsource")

find_package(Qt6 REQUIRED COMPONENTS Core Positioning Widgets)

qt_standard_project_setup()

qt_add_executable(logfilepositionsource
    clientapplication.cpp clientapplication.h
    logfilepositionsource.cpp logfilepositionsource.h
    main.cpp
)

set_target_properties(logfilepositionsource PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(logfilepositionsource PRIVATE
    Qt::Core
    Qt::Positioning
    Qt::Widgets
)

# Resources:
set(logfile_resource_files
    "simplelog.txt"
)

qt_add_resources(logfilepositionsource "logfile"
    PREFIX
        "/"
    FILES
        ${logfile_resource_files}
)

install(TARGETS logfilepositionsource
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
