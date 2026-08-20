# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16...3.21)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build (and the only way for iOS)
    project(tst_qlowenergycontroller_peripheral LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Test Gui)

    qt_add_executable(tst_qlowenergycontroller_peripheral
        tst_qlowenergycontroller_peripheral.cpp
    )
    target_link_libraries(tst_qlowenergycontroller_peripheral
        PUBLIC
            Qt::Core
            Qt::Bluetooth
            Qt::Test
            Qt::Gui
    )

else()

    qt_internal_add_test(tst_qlowenergycontroller_peripheral
        SOURCES
            tst_qlowenergycontroller_peripheral.cpp
        LIBRARIES
            Qt::Bluetooth
    )

    qt_internal_extend_target(tst_qlowenergycontroller_peripheral
        CONDITION ANDROID AND NOT ANDROID_EMBEDDED
        DEFINES
            QT_ANDROID_BLUETOOTH
    )

    if(MACOS)
        # Explicitly link against the static permission plugin because tests
        # currently don't have finalizers run for them except for iOS.
        # TODO: Remove this when qtbase automatically runs finalizers for tests: QTBUG-112212
        target_link_libraries(tst_qlowenergycontroller_peripheral
                              PRIVATE Qt6::QDarwinBluetoothPermissionPlugin)
    endif()

endif()

set_target_properties(tst_qlowenergycontroller_peripheral PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../shared ABSOLUTE)
    if(IOS)
        set_target_properties(tst_qlowenergycontroller_peripheral PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(tst_qlowenergycontroller_peripheral PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
        )
    endif()
endif()
