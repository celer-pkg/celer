# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(codecs LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/core5/widgets/tools/codecs")

find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat Gui Widgets)

qt_add_executable(codecs
    encodingdialog.cpp encodingdialog.h
    main.cpp
    mainwindow.cpp mainwindow.h
    previewform.cpp previewform.h
)

set_target_properties(codecs PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(codecs PUBLIC
    Qt::Core
    Qt::Core5Compat
    Qt::Gui
    Qt::Widgets
)

# Resources:
set(codecs_resource_files
    "images/editcopy.png"
)

qt6_add_resources(codecs "codecs"
    PREFIX
        "/"
    FILES
        ${codecs_resource_files}
)

install(TARGETS codecs
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
