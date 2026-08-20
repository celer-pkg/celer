# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(spawn LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(test_spawn
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DPhysics
)

# Resources:
set(qml_resource_files
  "main.qml"
  "box.qml"
  "sphere.qml"
)

qt6_add_resources(test_spawn "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
