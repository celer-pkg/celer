# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qlowenergycontroller-gattserver Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qlowenergycontroller-gattserver LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qlowenergycontroller-gattserver
    SOURCES
        tst_qlowenergycontroller-gattserver.cpp
    LIBRARIES
        Qt::Bluetooth
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_linux_crypto_api
    DEFINES
        CONFIG_LINUX_CRYPTO_API
)

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_bluez_le
    DEFINES
        CONFIG_BLUEZ_LE
)

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION boot2qt
    DEFINES
        CHECK_CMAC_SUPPORT
)
