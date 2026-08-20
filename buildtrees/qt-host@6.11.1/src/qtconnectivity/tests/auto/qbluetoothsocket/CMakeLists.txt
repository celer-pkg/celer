# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothsocket Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qbluetoothsocket LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qbluetoothsocket
    SOURCES
        tst_qbluetoothsocket.cpp
    LIBRARIES
        Qt::BluetoothPrivate
        Qt::Network
)

#### Keys ignored in scope 1:.:.:qbluetoothsocket.pro:<TRUE>:
# OTHER_FILES = "README.txt"
# testcase.timeout = "250"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothsocket CONDITION MACOS
    DEFINES
        QT_OSX_BLUETOOTH
    LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothsocket CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)
