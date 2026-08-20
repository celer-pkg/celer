# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(convexmesh LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(test_convexmesh
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DPhysics
)

# Resources:
set(qml_resource_files
  "main.qml"
)

qt6_add_resources(test_convexmesh "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


qt6_add_resources(test_convexmesh "assets"
    PREFIX
        "/"
    FILES
        "qt_logo_rect.png"
        "meshes/test.mesh"
        "meshes/field.mesh"
        "meshes/newTorus.mesh"
        "meshes/newConvexTorus.mesh"
        "maps/heightfield.png"
)
