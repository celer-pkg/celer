# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_rhirendering
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::GuiPrivate
        Qt::Quick3DPrivate
)

qt_add_qml_module(manual_test_rhirendering
    VERSION
        1.0
    URI
        QtQuick3DTest.RhiRendering
    QML_FILES
        main.qml
    SOURCES
        rhirenderingextension.cpp rhirenderingextension.h
    RESOURCE_PREFIX
        /
)

qt_add_shaders(manual_test_rhirendering "rhirendering_shaders"
    PREFIX
        /
    FILES
        shaders/solidcolor.vert
        shaders/solidcolor.frag
)
