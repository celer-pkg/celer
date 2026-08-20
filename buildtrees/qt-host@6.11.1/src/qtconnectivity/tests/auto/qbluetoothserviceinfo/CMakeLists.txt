# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothserviceinfo Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qbluetoothserviceinfo LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qbluetoothserviceinfo
    SOURCES
        tst_qbluetoothserviceinfo.cpp
    LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)
