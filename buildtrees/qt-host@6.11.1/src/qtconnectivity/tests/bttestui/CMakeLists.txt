# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## bttestui Binary:
#####################################################################

cmake_minimum_required(VERSION 3.16...3.21)

# Resources:
set(bttest_resource_files
    "Button.qml"
    "main.qml"
)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build
    project(bttestui LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Quick)

    qt_add_executable(bttestui
            btlocaldevice.cpp btlocaldevice.h
            main.cpp
    )

    target_link_libraries(bttestui PUBLIC
        Qt::Bluetooth
        Qt::Gui
        Qt::Quick
    )

    qt_add_resources(bttestui "bttest"
        PREFIX
            "/"
        FILES
            ${bttest_resource_files}
    )
else()
    # in-tree build
    qt_internal_add_executable(bttestui
        GUI
        SOURCES
            btlocaldevice.cpp btlocaldevice.h
            main.cpp
        LIBRARIES
            Qt::Bluetooth
            Qt::Gui
            Qt::Quick
    )

    qt_internal_add_resource(bttestui "bttest"
        PREFIX
            "/"
        FILES
            ${bttest_resource_files}
    )

    if(MACOS)
        # Explicitly link against the static permission plugin because tests
        # currently don't have finalizers run for them except for iOS.
        # TODO: Remove this when qtbase automatically runs finalizers for tests: QTBUG-112212
        target_link_libraries(bttestui PRIVATE Qt6::QDarwinBluetoothPermissionPlugin)
    endif()

endif()

set_target_properties(bttestui PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if(IOS)
        set_target_properties(bttestui PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(bttestui PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
        )
    endif()
endif()


