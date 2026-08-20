# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_standard_project_setup(REQUIRES 6.8)

if(NOT QT_FEATURE_developer_build)
    message(WARNING "Skipping manual test shaderPoker, requires developer build")
    return()
endif()

qt_internal_add_manual_test(manual_test_shaderPoker
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DGlslParserPrivate
)

qt_add_qml_module(manual_test_shaderPoker
    URI io.qt.tests.manual.shaderpoker
    VERSION 1.0
    QML_FILES
        Main.qml
        ShaderEditor.qml
        SectionLayout.qml
        VertexColorMaskEditor.qml
        VertexAttributeEditor.qml
        ImageMapEditor.qml
    SOURCES
        shaderpoker.h shaderpoker.cpp
        syntaxhighlighter.h syntaxhighlighter.cpp
        defaultmaterialshaderproperties.h defaultmaterialshaderproperties.cpp
    RESOURCES
        images/TreeExpanded.png
        images/TreeExpanded@2x.png
        images/TreeExpanded@3x.png
        images/TreeExpanded@4x.png
        images/TreeUnexpanded.png
        images/TreeUnexpanded@2x.png
        images/TreeUnexpanded@3x.png
        images/TreeUnexpanded@4x.png
    NO_CACHEGEN
)
