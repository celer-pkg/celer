# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(heartrate-server LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/heartrate-server")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core)

qt_standard_project_setup()

if(ANDROID OR APPLE)
    find_package(Qt6 REQUIRED COMPONENTS Gui)
endif()

qt_add_executable(heartrate-server
    main.cpp
)

set_target_properties(heartrate-server PROPERTIES
    MACOSX_BUNDLE TRUE
)

target_link_libraries(heartrate-server PRIVATE
    Qt6::Bluetooth
    Qt6::Core
)

if(ANDROID OR APPLE)
    target_link_libraries(heartrate-server PRIVATE
        Qt6::Gui
    )
endif()

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(heartrate-server PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(heartrate-server PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

install(TARGETS heartrate-server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
