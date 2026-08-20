# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(desktop_vr_hybrid_app LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Quick3D Quick3DXr ShaderTools)

qt_internal_add_manual_test(desktop_vr_hybrid_app
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DXr
)

qt_add_resources(desktop_vr_hybrid_app "qml"
    PREFIX
        "/"
    FILES
        "main.qml"
        "ActionMapper.qml"
        "VrScene.qml"
        "LeftHand.qml"
        "RightHand.qml"
        "Lazer.qml"
        "Beam.qml"
        "Some3DScene.qml"
        "lazer.mesh"
        "Lazer_baseColor.png"
        "Lazer_metalness.png"
)
