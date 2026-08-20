# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qmlaxishandling LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS DataVisualization)

qt_add_executable(qmlaxishandling
    main.cpp
)
set_target_properties(qmlaxishandling PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(qmlaxishandling PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::DataVisualization
)

qt6_add_qml_module(qmlaxishandling
    URI AxisHandling
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    SOURCES
        customformatter.cpp customformatter.h
    QML_FILES
        qml/qmlaxishandling/main.qml
        qml/qmlaxishandling/Data.qml
        qml/qmlaxishandling/AxisDragging.qml
        qml/qmlaxishandling/AxisFormatting.qml
    RESOURCES
        qml/qmlaxishandling/cube.obj
        qml/qmlaxishandling/cubetexture.png
)

install(TARGETS qmlaxishandling
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
