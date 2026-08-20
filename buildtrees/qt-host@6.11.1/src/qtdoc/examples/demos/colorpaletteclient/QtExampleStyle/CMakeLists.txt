# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtexamplestyle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

set_source_files_properties(UIStyle.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_policy(SET QTP0001 NEW)
qt_add_qml_module(qtexamplestyle
    URI QtExampleStyle
    PLUGIN_TARGET qtexamplestyle
    QML_FILES
        Button.qml
        Popup.qml
        UIStyle.qml
        TextField.qml
)

target_link_libraries(qtexamplestyle PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

if(UNIX AND NOT APPLE AND CMAKE_CROSSCOMPILING)
    find_package(Qt6 REQUIRED COMPONENTS QuickTemplates2)

    # Work around QTBUG-86533
    target_link_libraries(qtexamplestyle PRIVATE Qt6::QuickTemplates2)
endif()

install(TARGETS qtexamplestyle
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/QtExampleStyle"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/QtExampleStyle"
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    DESTINATION "${CMAKE_INSTALL_BINDIR}/QtExampleStyle"
)
