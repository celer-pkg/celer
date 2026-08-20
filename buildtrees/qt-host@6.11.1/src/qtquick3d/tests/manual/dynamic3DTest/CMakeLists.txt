# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_dynamic3DTest
    GUI
    SOURCES
        main.cpp
        gradienttexture.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

set(qml_resource_files
    "main.qml"
    "RiggedFigure.qml"
    "random1.mesh"
    "random2.mesh"
    "random3.mesh"
    "random4.mesh"
    "noise1.jpg"
    "noise2.jpg"
    "noise3.jpg"
    "noise4.jpg"
    "noise5.jpg"
    "riggedfigure.mesh"
)

qt_internal_add_resource(manual_test_dynamic3DTest "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

qt_add_qml_module(manual_test_dynamic3DTest
    VERSION 1.0
    URI io.qt.tests.manual.dynamic3DTest
    RESOURCE_PREFIX /
)
