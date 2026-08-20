# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_volumetrictest LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_volumetrictest
    GUI
    SOURCES
        main.cpp
        volumetrictest.cpp volumetrictest.h
    )

target_link_libraries(tst_volumetrictest PUBLIC
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
    )

set_source_files_properties("cubeFilledFlat.mesh"
    PROPERTIES QT_RESOURCE_ALIAS "mesh"
    )
set(volumetrictest_resource_files
    "cubeFilledFlat.mesh"
    "logo.png"
    "logo_no_padding.png"
    )

qt_internal_add_resource(tst_volumetrictest "volumetrictest"
    PREFIX
    "/"
    FILES
    ${volumetrictest_resource_files}
    )
