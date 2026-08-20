# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries

qt_find_package(BlueZ PROVIDED_TARGETS PkgConfig::BLUEZ)


#### Tests

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez/CMakeLists.txt")
    qt_config_compile_test("bluez"
                           LABEL "BlueZ"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez_le/CMakeLists.txt")
    qt_config_compile_test("bluez_le"
                           LABEL "BlueZ Low Energy"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez_le")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/linux_crypto_api/CMakeLists.txt")
    qt_config_compile_test("linux_crypto_api"
                           LABEL "Linux Crypto API"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/linux_crypto_api")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt_bt/CMakeLists.txt")
    qt_config_compile_test("winrt_bt"
                           LABEL "WinRT Bluetooth API"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt_bt")
endif()


#### Features

qt_feature("bluez" PUBLIC
    LABEL "BlueZ"
    CONDITION BLUEZ_FOUND AND TEST_bluez AND QT_FEATURE_dbus
)
qt_feature("bluez_le" PRIVATE
    LABEL "BlueZ Low Energy"
    CONDITION QT_FEATURE_bluez AND TEST_bluez_le
)
qt_feature("linux_crypto_api" PRIVATE
    LABEL "Linux Crypto API"
    CONDITION QT_FEATURE_bluez_le AND TEST_linux_crypto_api
)
qt_feature("winrt_bt" PRIVATE
    LABEL "WinRT Bluetooth API"
    CONDITION WIN32 AND TEST_winrt_bt
)

qt_configure_add_summary_section(NAME "Qt Bluetooth details")
qt_configure_add_summary_entry(ARGS bluez)
qt_configure_add_summary_entry(ARGS bluez_le)
qt_configure_add_summary_entry(ARGS linux_crypto_api)
qt_configure_add_summary_entry(ARGS winrt_bt)
qt_configure_end_summary_section()
qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "Bluez version is too old to support Bluetooth Low Energy. Only classic Bluetooth will be available."
    CONDITION QT_FEATURE_bluez AND NOT QT_FEATURE_bluez_le
)
qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "Linux crypto API not present. BTLE signed writes will not work."
    CONDITION QT_FEATURE_bluez_le AND NOT QT_FEATURE_linux_crypto_api
)
