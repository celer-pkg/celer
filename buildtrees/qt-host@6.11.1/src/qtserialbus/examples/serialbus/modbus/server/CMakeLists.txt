# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(modbusserver LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/modbus/server")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network SerialBus Widgets)

qt_standard_project_setup()

qt_add_executable(modbusserver
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    settingsdialog.cpp settingsdialog.h settingsdialog.ui
)

set_target_properties(modbusserver PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(modbusserver PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::SerialBus
    Qt::Widgets
)

# Resources:
set(server_resource_files
    "images/application-exit.png"
    "images/connect.png"
    "images/disconnect.png"
    "images/settings.png"
)

qt_add_resources(modbusserver "server"
    PREFIX
        "/"
    FILES
        ${server_resource_files}
)

if(QT_FEATURE_modbus_serialport)
    target_link_libraries(modbusserver PRIVATE
        Qt::SerialPort
    )
endif()

install(TARGETS modbusserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
