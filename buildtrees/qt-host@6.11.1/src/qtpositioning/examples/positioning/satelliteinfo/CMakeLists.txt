# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(satelliteinfo LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/positioning/satelliteinfo")

find_package(Qt6 REQUIRED COMPONENTS Core Svg Quick PositioningQuick)
find_package(Qt6 OPTIONAL_COMPONENTS SerialPort)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(satelliteinfo
    main.cpp
)

set_target_properties(satelliteinfo PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.satelliteinfo"
)

target_link_libraries(satelliteinfo PRIVATE
    Qt::Core
    Qt::PositioningQuick
    Qt::Quick
    Qt::Svg
)

# For mobile platforms we need to explicitly link with Qt::SerialPort
# to use NMEA plugin. The reason for such behavior is that, at least on Android,
# the plugin dependencies were not explicitly added to the list of deployment
# targets until Qt 6.5.
if(ANROID AND TARGET Qt::SerialPort)
    target_link_libraries(satelliteinfo PRIVATE Qt::SerialPort)
endif()

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(satelliteinfo PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(satelliteinfo PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

qt_add_resources(satelliteinfo "resources"
    FILES
        nmealog.txt
        fonts/TitilliumWeb-Regular.ttf
        fonts/TitilliumWeb-SemiBold.ttf
)

set_source_files_properties(Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(satelliteinfo
    URI SatelliteInformation
    VERSION 1.0
    SOURCES
        roles.h
        satellitemodel.cpp satellitemodel.h
        sortfiltermodel.cpp sortfiltermodel.h
    QML_FILES
        ApplicationScreen.qml
        Button.qml
        Header.qml
        HelpPopup.qml
        LegendBox.qml
        Main.qml
        RssiView.qml
        PageButton.qml
        PermissionsScreen.qml
        PositionBox.qml
        SatelliteView.qml
        SettingsView.qml
        SkyView.qml
        Theme.qml
        ViewSwitch.qml
    RESOURCES
        icons/checkbox.svg
        icons/checkbox_blank.svg
        icons/darkmode.svg
        icons/filter.svg
        icons/help.svg
        icons/lightmode.svg
        icons/place.svg
        icons/qtlogo_green.png
        icons/qtlogo_white.png
        icons/rssiview.svg
        icons/satellite_small.png
        icons/satellite1.png
        icons/satellite2.png
        icons/search.svg
        icons/settings.svg
        icons/skyview.svg
        icons/sort.svg
        icons/tableview.svg
)

install(TARGETS satelliteinfo
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

if(APPLE AND NOT CMAKE_GENERATOR STREQUAL "Xcode")
    # Need to sign application for location permissions to work
    add_custom_command(TARGET satelliteinfo
        POST_BUILD COMMAND codesign -s - satelliteinfo.app)
endif()
