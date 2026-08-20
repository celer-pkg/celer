# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
     cmake_minimum_required(VERSION 3.16)
     project(btscanner LANGUAGES CXX)
     find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
 endif()

qt_internal_add_manual_test(btscanner
    GUI
    SOURCES
        device.cpp device.h device.ui
        main.cpp
        service.cpp service.h service.ui
    LIBRARIES
        Qt::Bluetooth
        Qt::Core
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

if(MACOS)
    # Explicitly link against the static permission plugin because tests
    # currently don't have finalizers run for them except for iOS.
    # TODO: Remove this when qtbase automatically runs finalizers for tests: QTBUG-112212
    target_link_libraries(btscanner PRIVATE Qt6::QDarwinBluetoothPermissionPlugin)
endif()

set_target_properties(btscanner PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    if (IOS)
        set_target_properties(btscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        )
    else()
        # Using absolute path for shared plist files is a Ninja bug workaround
        get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../examples/bluetooth/shared ABSOLUTE)
        set_target_properties(btscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()
