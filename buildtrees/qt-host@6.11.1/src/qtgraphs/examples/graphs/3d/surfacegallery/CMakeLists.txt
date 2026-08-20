# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(surfacegallery LANGUAGES CXX)

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
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Quick3D)
find_package(Qt6 COMPONENTS Graphs)

qt_add_executable(surfacegallery
    main.cpp
)
set_target_properties(surfacegallery PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(surfacegallery PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
    Qt::Graphs
)

qt6_add_qml_module(surfacegallery
    URI SurfaceGalleryExample
    NO_RESOURCE_TARGET_PATH
    SOURCES
        datasource.cpp datasource.h
    QML_FILES
        qml/surfacegallery/main.qml
        qml/surfacegallery/SpectrogramData.qml
        qml/surfacegallery/SurfaceHeightMap.qml
        qml/surfacegallery/SurfaceOscilloscope.qml
        qml/surfacegallery/SurfaceSpectrogram.qml
    RESOURCES
        qml/surfacegallery/heightmap.png
)

install(TARGETS surfacegallery
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
