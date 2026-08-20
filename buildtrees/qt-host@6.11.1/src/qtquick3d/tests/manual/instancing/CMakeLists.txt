# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_instancing
    GUI
    SOURCES
        main.cpp
        customgeometry.cpp customgeometry.h
        custominstancing.cpp custominstancing.h
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

set(qml_resource_files
    "main.qml"
    "material.frag"
    "material.vert"
    "test.xml"
)

qt_internal_add_resource(manual_test_instancing "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

qt_add_qml_module(manual_test_instancing
    VERSION 1.0
    URI InstancingTest
    RESOURCE_PREFIX /
)
