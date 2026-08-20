# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(ANDROID OR WINRT)
    message( FATAL_ERROR "This example is not supported for android or winrt. CMake will exit." )
endif()

cmake_minimum_required(VERSION 3.16)
project(widgetvolumetric LANGUAGES CXX)

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
find_package(Qt6 COMPONENTS Graphs)
find_package(Qt6 COMPONENTS GraphsWidgets)

qt_add_executable(widgetvolumetric
    main.cpp
    volumetric.cpp volumetric.h
)
set_target_properties(widgetvolumetric PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(widgetvolumetric PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
)

if (EMSCRIPTEN)
target_link_options(widgetvolumetric PRIVATE
    -sFULL_ES3=1
    -sNO_DISABLE_EXCEPTION_CATCHING
)
endif()

set(widgetvolumetric_resource_files
    "layer_ground.png"
    "layer_magma.png"
    "layer_water.png"
)

qt6_add_resources(widgetvolumetric "widgetvolumetric"
    PREFIX
        "/heightmaps"
    FILES
        ${widgetvolumetric_resource_files}
)

install(TARGETS widgetvolumetric
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
