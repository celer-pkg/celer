# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_textureprovider
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

qt_add_qml_module(manual_test_textureprovider
    VERSION
        1.0
    URI
        QtQuick3DTest.Extensions.TextureProvider
    QML_FILES
        Main.qml
    SOURCES
        renderextension.cpp renderextension.h
)

qt_add_shaders(manual_test_textureprovider "textureprovider_shaders"
    PREFIX
        /
    FILES
        shaders/color.vert
        shaders/color.frag
)

qt_add_resources(manual_test_textureprovider "textureprovider_resources"
    PREFIX
        /QtQuick3DTest/Extensions/TextureProvider
    FILES
        materials/unshaded_custommaterial.vert
        materials/unshaded_custommaterial.frag
        materials/shaded_custommaterial.frag
        effects/effect.frag
)
