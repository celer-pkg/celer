# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_bleclient LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_internal_add_manual_test(tst_bleclient
    SOURCES
        ../common/bleiodevice.cpp ../common/bleiodevice.h
        connectpage.cpp connectpage.h connectpage.ui
        mainwindow.cpp mainwindow.h
        heaterview.cpp heaterview.h heaterview.ui
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Widgets
        Qt::Bluetooth
        Qt::RemoteObjects
)

qt_autogen_tools(tst_bleclient ENABLE_AUTOGEN_TOOLS moc uic)

set_target_properties(tst_bleclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_include_directories(tst_bleclient PUBLIC
    ../common
)

qt_add_repc_replicas(tst_bleclient
    ../common/heater.rep
)

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common ABSOLUTE)
    if (IOS)
        set_target_properties(tst_bleclient PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(tst_bleclient PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()
