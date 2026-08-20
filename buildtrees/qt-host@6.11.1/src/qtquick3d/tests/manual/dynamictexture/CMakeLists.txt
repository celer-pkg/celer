# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_dynamictexture
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

qt_add_qml_module(manual_test_dynamictexture
    URI Example
    VERSION 1.0
    QML_FILES
        CorkBoards.qml
        Doors.qml
        main.qml
        content/Panel.qml
    RESOURCES
        meshes/door1.mesh
        meshes/door2.mesh
        meshes/wall.mesh
        content/cork.jpg
        content/note-yellow.png
        content/tack.png
    NO_RESOURCE_TARGET_PATH
)

