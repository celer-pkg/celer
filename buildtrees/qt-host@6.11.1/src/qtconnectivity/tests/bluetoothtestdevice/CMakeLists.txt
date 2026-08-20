# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## bluetoothtestdevice Tool:
#####################################################################

cmake_minimum_required(VERSION 3.16...3.21)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build
    project(bluetoothtestdevice LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core)
    if(ANDROID OR APPLE)
        find_package(Qt6 REQUIRED COMPONENTS Gui)
    endif()

    qt_add_executable(
        bluetoothtestdevice
            bluetoothtestdevice.cpp
    )

else()

    qt_internal_add_executable(bluetoothtestdevice
        SOURCES
            bluetoothtestdevice.cpp
    )

    if(MACOS)
        # Explicitly link against the static permission plugin because tests
        # currently don't have finalizers run for them except for iOS.
        # TODO: Remove this when qtbase automatically runs finalizers for tests: QTBUG-112212
        target_link_libraries(bluetoothtestdevice PRIVATE Qt6::QDarwinBluetoothPermissionPlugin)
    endif()

endif()

set_target_properties(bluetoothtestdevice PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if(IOS)
        set_target_properties(bluetoothtestdevice PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(bluetoothtestdevice PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
        )
    endif()
endif()

target_link_libraries(
        bluetoothtestdevice
    PUBLIC
        Qt::Core
        Qt::Bluetooth
)

if(ANDROID OR APPLE)
    target_link_libraries(
            bluetoothtestdevice
            PUBLIC
            Qt::Gui
    )
endif()
