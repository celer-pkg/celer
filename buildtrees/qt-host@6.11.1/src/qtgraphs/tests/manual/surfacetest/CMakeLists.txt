# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_surfacetest LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_surfacetest
    GUI
    SOURCES
        buttonwrapper.cpp buttonwrapper.h
        checkboxwrapper.cpp checkboxwrapper.h
        graphmodifier.cpp graphmodifier.h
        main.cpp
    )
target_link_libraries(tst_surfacetest PUBLIC
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
    )

set_source_files_properties("Heightmap.png"
    PROPERTIES QT_RESOURCE_ALIAS "map"
    )
set_source_files_properties("mapimage.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapimage"
    )
set_source_files_properties("opacitymask.png"
    PROPERTIES QT_RESOURCE_ALIAS "opacitymask"
    )
set(surfacetest_resource_files
    "Heightmap.png"
    "mapimage.png"
    "opacitymask.png"
    )

qt_internal_add_resource(tst_surfacetest "surfacetest"
    PREFIX
    "/maps"
    FILES
    ${surfacetest_resource_files}
    )
