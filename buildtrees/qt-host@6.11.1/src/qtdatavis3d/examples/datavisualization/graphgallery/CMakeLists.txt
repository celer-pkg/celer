# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(graphgallery LANGUAGES CXX)

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
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS DataVisualization)

qt_add_executable(graphgallery
    main.cpp
    bargraph.cpp bargraph.h
    graphmodifier.cpp graphmodifier.h
    rainfalldata.cpp rainfalldata.h
    variantbardatamapping.cpp variantbardatamapping.h
    variantbardataproxy.cpp variantbardataproxy.h
    variantdataset.cpp variantdataset.h
    scattergraph.cpp scattergraph.h
    scatterdatamodifier.cpp scatterdatamodifier.h
    axesinputhandler.cpp axesinputhandler.h
    surfacegraph.cpp surfacegraph.h
    surfacegraphmodifier.cpp surfacegraphmodifier.h
    custominputhandler.cpp custominputhandler.h
    highlightseries.cpp highlightseries.h
    topographicseries.cpp topographicseries.h
)
set_target_properties(graphgallery PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(graphgallery PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::DataVisualization
)

set(graphgallery_resource_files
    "data/raindata.txt"
    "data/layer_1.png"
    "data/layer_2.png"
    "data/layer_3.png"
    "data/oilrig.obj"
    "data/pipe.obj"
    "data/refinery.obj"
    "data/maptexture.jpg"
    "data/topography.png"
)

qt6_add_resources(graphgallery "graphgallery"
    PREFIX
        "/"
    FILES
        ${graphgallery_resource_files}
)

install(TARGETS graphgallery
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
