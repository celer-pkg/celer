# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(can LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/can")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network SerialBus Widgets)

qt_standard_project_setup()

qt_add_executable(can
    bitratebox.cpp bitratebox.h
    canbusdeviceinfobox.h canbusdeviceinfobox.cpp canbusdeviceinfobox.ui
    canbusdeviceinfodialog.h canbusdeviceinfodialog.cpp canbusdeviceinfodialog.ui
    connectdialog.cpp connectdialog.h connectdialog.ui
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    sendframebox.cpp sendframebox.h sendframebox.ui
    receivedframesmodel.cpp receivedframesmodel.h
    receivedframesview.cpp receivedframesview.h
)

set_target_properties(can PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(can PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::SerialBus
    Qt::Widgets
)

# Resources:
set(can_resource_files
    "images/application-exit.png"
    "images/clear.png"
    "images/connect.png"
    "images/disconnect.png"
)

qt_add_resources(can "can"
    PREFIX
        "/"
    FILES
        ${can_resource_files}
)

install(TARGETS can
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
