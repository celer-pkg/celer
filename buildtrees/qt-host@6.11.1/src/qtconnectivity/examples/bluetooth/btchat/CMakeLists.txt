# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(btchat LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/btchat")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Widgets)

qt_standard_project_setup()

qt_add_executable(btchat
    chat.cpp chat.h chat.ui
    chatclient.cpp chatclient.h
    chatserver.cpp chatserver.h
    main.cpp
    remoteselector.cpp remoteselector.h remoteselector.ui
)

set(icon_files)
foreach(icon IN ITEMS bluetooth bluetooth_dark send send_dark)
    foreach(scale IN ITEMS "" "@2" "@3" "@4")
        list(APPEND icon_files "icons/btchat/24x24${scale}/${icon}.png")
    endforeach()
endforeach()

qt_add_resources(btchat "theme" FILES
    ${icon_files}
    icons/btchat/index.theme
)

set_target_properties(btchat PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(btchat PRIVATE
    Qt6::Bluetooth
    Qt6::Core
    Qt6::Widgets
)

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(btchat PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(btchat PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

install(TARGETS btchat
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
