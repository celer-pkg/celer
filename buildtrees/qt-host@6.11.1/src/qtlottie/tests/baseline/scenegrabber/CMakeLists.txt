# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qmlscenegrabber Binary:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(qtlottie_qmlscenegrabber LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_executable(qtlottie_qmlscenegrabber
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
)

qt_policy(SET QTP0001 NEW)
qt_add_qml_module(qtlottie_qmlscenegrabber
    URI QtLottieAnimation
    QML_FILES
    "Main.qml"
)

qt_add_resources(qtlottie_qmlscenegrabber "qtlottie_qmlscenegrabber"
    PREFIX
        "/"
    FILES
        checkered.png
)
