# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(heartrate-game LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/heartrate-game")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(heartrate-game
    heartrate-global.h
    main.cpp
)

set_target_properties(heartrate-game PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(heartrate-game PRIVATE
    Qt6::Bluetooth
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

set_source_files_properties(GameSettings.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(heartrate-game
    URI HeartRateGame
    VERSION 1.0
    SOURCES
        bluetoothbaseclass.cpp bluetoothbaseclass.h
        connectionhandler.cpp connectionhandler.h
        devicefinder.cpp devicefinder.h
        devicehandler.cpp devicehandler.h
        deviceinfo.cpp deviceinfo.h
    QML_FILES
        App.qml
        BluetoothAlarmDialog.qml
        BottomLine.qml
        Connect.qml
        GameButton.qml
        GamePage.qml
        GameSettings.qml
        Measure.qml
        SplashScreen.qml
        Stats.qml
        StatsLabel.qml
        TitleBar.qml
        Main.qml
    RESOURCES
        images/alert.svg
        images/bluetooth.svg
        images/bt_off_to_on.png
        images/clock.svg
        images/heart.png
        images/logo.png
        images/progress.svg
        images/search.svg
)

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(heartrate-game PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(heartrate-game PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

install(TARGETS heartrate-game
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
