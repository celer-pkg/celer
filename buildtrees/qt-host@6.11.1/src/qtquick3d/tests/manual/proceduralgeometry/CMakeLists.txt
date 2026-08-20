# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_standard_project_setup(REQUIRES 6.8)

qt_internal_add_manual_test(manual_test_proceduralgeometry
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

qt_add_qml_module(manual_test_proceduralgeometry
    URI io.qt.tests.manual.proceduralgeometry
    VERSION 1.0
    QML_FILES
        Main.qml
        OriginGizmo.qml
        ExtrudedTextSettings.qml
        TorusSettings.qml
        ConeSettings.qml
        CylinderSettings.qml
        SphereSettings.qml
        PlaneSettings.qml
        CuboidSettings.qml
        CapsuleSettings.qml
        UVChecker.qml
)
