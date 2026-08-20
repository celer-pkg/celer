# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_rotations LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_rotations
    GUI
    SOURCES
        main.cpp
        scatterdatamodifier.cpp scatterdatamodifier.h
        magneticfield.cpp magneticfield.h
        customitemrotation.cpp customitemrotation.h
        customitemdatamodifier.cpp customitemdatamodifier.h
    )

target_link_libraries(tst_rotations PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
)

set(rotations_resource_files
    "mesh/largesphere.mesh"
    "mesh/narrowarrow.mesh"
)

qt6_add_resources(tst_rotations "rotations"
    PREFIX
        "/"
    FILES
        ${rotations_resource_files}
)
