# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(lowenergyscanner LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/lowenergyscanner")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(lowenergyscanner
    main.cpp
)

set_target_properties(lowenergyscanner PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(lowenergyscanner PUBLIC
    Qt6::Bluetooth
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(lowenergyscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(lowenergyscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

qt_add_qml_module(lowenergyscanner
    URI Scanner
    VERSION 1.0
    SOURCES
        characteristicinfo.cpp characteristicinfo.h
        device.cpp device.h
        deviceinfo.cpp deviceinfo.h
        serviceinfo.cpp serviceinfo.h
    QML_FILES
        Characteristics.qml
        Devices.qml
        Dialog.qml
        Header.qml
        Label.qml
        Main.qml
        Menu.qml
        Services.qml
    RESOURCES
        assets/busy_dark.png
)

install(TARGETS lowenergyscanner
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
