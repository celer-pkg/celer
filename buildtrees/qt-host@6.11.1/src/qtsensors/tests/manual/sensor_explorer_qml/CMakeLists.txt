# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(tst_manual_sensor_explorer_qml LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Quick Sensors)

qt_add_executable(tst_manual_sensor_explorer_qml
    main.cpp
    sensormodels.cpp sensormodels.h
)

set_target_properties(tst_manual_sensor_explorer_qml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(tst_manual_sensor_explorer_qml PUBLIC
    Qt::Qml
    Qt::Quick
    Qt::Sensors
)

qt_add_qml_module(tst_manual_sensor_explorer_qml
    URI SensorModels
    VERSION 1.0
    QML_FILES sensor_explorer.qml
    NO_RESOURCE_TARGET_PATH
)
