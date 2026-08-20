# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_bakedprimitives
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DHelpers
        Qt::Quick3DPrivate
)

# Resources:
set(qml_resource_files
    "main.qml"
    "SettingsPane.qml"
)

qt_internal_add_resource(manual_test_bakedprimitives "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
