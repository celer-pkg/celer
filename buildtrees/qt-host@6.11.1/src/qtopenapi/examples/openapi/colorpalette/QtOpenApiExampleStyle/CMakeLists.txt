# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtopenapi_examplestyle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

set_source_files_properties(UIStyle.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_policy(SET QTP0001 NEW)
qt_add_qml_module(qtopenapi_examplestyle
    URI QtOpenApiExampleStyle
    PLUGIN_TARGET qtopenapi_examplestyle
    QML_FILES
        Button.qml
        Popup.qml
        UIStyle.qml
        TextField.qml
        ToolBar.qml
        ToolButton.qml
        Label.qml
)

target_link_libraries(qtopenapi_examplestyle PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

if(UNIX AND NOT APPLE AND CMAKE_CROSSCOMPILING)
    find_package(Qt6 REQUIRED COMPONENTS QuickTemplates2)

    # Work around QTBUG-86533
    target_link_libraries(qtopenapi_examplestyle PRIVATE Qt6::QuickTemplates2)
endif()

install(TARGETS qtopenapi_examplestyle
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/QtOpenApiExampleStyle"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/QtOpenApiExampleStyle"
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    DESTINATION "${CMAKE_INSTALL_BINDIR}/QtOpenApiExampleStyle"
)
