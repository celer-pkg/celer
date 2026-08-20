# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(mediaplayer LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/mediaplayer")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Scxml)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(mediaplayerscxml
    main.cpp
)

set_target_properties(mediaplayerscxml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(mediaplayerscxml PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Scxml
)

# Statecharts:
qt6_add_statecharts(mediaplayerscxml
    mediaplayer.scxml
)

qt_add_qml_module(mediaplayerscxml
    URI Mediaplayer
    VERSION 1.0
    QML_FILES
        MainWindow.qml
    SOURCES
        mediaplayer-qml.h
        thedatamodel.h thedatamodel.cpp
)

install(TARGETS mediaplayerscxml
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
