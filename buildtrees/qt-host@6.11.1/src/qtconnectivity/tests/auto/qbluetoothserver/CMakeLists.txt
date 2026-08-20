# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothserver Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qbluetoothserver LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qbluetoothserver
    SOURCES
        tst_qbluetoothserver.cpp
    LIBRARIES
        Qt::BluetoothPrivate
)

#### Keys ignored in scope 1:.:.:qbluetoothserver.pro:<TRUE>:
# OTHER_FILES = "README.txt"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserver CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)
