# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_androidassets LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_androidassets
    SOURCES
        tst_androidassets.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Quick
)

if(ANDROID)
    # TODO: avoid using private API, introduce the public variant instead. See QTBUG-135973.
    _qt_internal_android_get_target_deployment_dir(android_build_dir tst_androidassets)
else()
    set(android_build_dir "${CMAKE_CURRENT_BINARY_DIR}/android-build")
endif()

# add qml/*.qml files as assets instead of resources
file(
    COPY qml/main.qml
    DESTINATION "${android_build_dir}/assets/qml/")

file(
    COPY qml/pages/MainPage.qml
    DESTINATION "${android_build_dir}/assets/qml/pages/")
